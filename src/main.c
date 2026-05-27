#include <stdio.h>
#include "ring_buffer.h"
#include "parser.h"

// Globally shared ring buffer instance
ring_buffer_t uart_rx_buffer;

/**
 * SIMULATED HARDWARE INTERRUPT SERVICE ROUTINE (ISR)
 * In a real microcontroller (like an STM32 or Arduino), this function 
 * fires automatically at the hardware level whenever a byte arrives over the rx pin.
 */
void Simulated_UART_RX_ISR(uint8_t incoming_byte) {
    // Hardware pushes the byte into our FIFO ring buffer completely in the background
    ring_buffer_queue(&uart_rx_buffer, incoming_byte);
}

/**
 * HELPER FUNCTION: Simulates sending an entire frame byte-by-byte
 */
void simulate_incoming_packet(const uint8_t *payload, uint8_t length) {
    printf("\n--- Simulating Incoming Serial Transmission ---\n");
    
    // 1. Send Start of Frame (SOF)
    Simulated_UART_RX_ISR(SOF_BYTE);
    
    // 2. Send Packet Length
    Simulated_UART_RX_ISR(length);
    
    // 3. Send Payload bytes
    for (uint8_t i = 0; i < length; i++) {
        Simulated_UART_RX_ISR(payload[i]);
    }
    
    // 4. Calculate and Send CRC-16 Checksum (High byte then Low byte)
    uint16_t crc = crc16_ccitt(payload, length);
    Simulated_UART_RX_ISR((uint8_t)(crc >> 8));   // High byte
    Simulated_UART_RX_ISR((uint8_t)(crc & 0xFF));  // Low byte
    
    // 5. Send End of Frame (EOF)
    Simulated_UART_RX_ISR(EOF_BYTE);
}

int main(void) {
    // Initialize our systems
    ring_buffer_init(&uart_rx_buffer);
    parser_init();
    
    printf("Embedded Serial Parser Simulation Initialized.\n");

    // TEST CASE 1: Send a perfectly healthy command packet
    // Payload: 'A', 'B', 'C' (3 bytes)
    uint8_t valid_payload[] = {0x41, 0x42, 0x43}; 
    simulate_incoming_packet(valid_payload, 3);
    
    // The Main Application Loop execution: Process whatever is in the buffer
    // This runs completely non-blocking!
    parser_process(&uart_rx_buffer);


    // TEST CASE 2: Send a packet with corrupted data (Simulating line noise)
    simulate_incoming_packet(valid_payload, 3);
    
    // Corrupt the data inside the buffer manually before the parser gets to it!
    // We sneakily change 'A' (0x41) to 'Z' (0x5A) inside the buffer data pool
    uart_rx_buffer.buffer[(uart_rx_buffer.tail + 2) % RING_BUFFER_SIZE] = 0x5A; 
    
    // Process the noisy frame
    parser_process(&uart_rx_buffer);

    return 0;
}
