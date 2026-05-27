#include "parser.h"
#include <stdio.h> // For debugging print statements

static parser_state_t current_state = STATE_WAIT_SOF;
static packet_t rx_packet;
static uint8_t payload_index = 0;
static uint8_t crc_high_byte = 0;

// Standard CRC-16-CCITT calculation (Polynomial: 0x1021)
uint16_t crc16_ccitt(const uint8_t *data, uint16_t length) {
    uint16_t crc = 0xFFFF; // Initial value
    for (uint16_t i = 0; i < length; i++) {
        crc ^= (uint16_t)data[i] << 8;
        for (uint8_t bit = 0; bit < 8; bit++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1021;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

void parser_init(void) {
    current_state = STATE_WAIT_SOF;
    payload_index = 0;
}

// Non-blocking parser processing loop
void parser_process(ring_buffer_t *rb) {
    uint8_t byte;

    // Process all bytes currently waiting in the ring buffer
    while (ring_buffer_dequeue(rb, &byte)) {
        switch (current_state) {
            
            case STATE_WAIT_SOF:
                if (byte == SOF_BYTE) {
                    payload_index = 0;
                    current_state = STATE_GET_LENGTH;
                }
                break;

            case STATE_GET_LENGTH:
                if (byte > MAX_PAYLOAD_SIZE || byte == 0) {
                    // Invalid length, reset state machine
                    current_state = STATE_WAIT_SOF;
                } else {
                    rx_packet.length = byte;
                    current_state = STATE_GET_PAYLOAD;
                }
                break;

            case STATE_GET_PAYLOAD:
                rx_packet.payload[payload_index++] = byte;
                if (payload_index >= rx_packet.length) {
                    current_state = STATE_GET_CRC_HIGH;
                }
                break;

            case STATE_GET_CRC_HIGH:
                crc_high_byte = byte;
                current_state = STATE_GET_CRC_LOW;
                break;

            case STATE_GET_CRC_LOW:
                rx_packet.crc = ((uint16_t)crc_high_byte << 8) | byte;
                current_state = STATE_WAIT_EOF;
                break;

            case STATE_WAIT_EOF:
                if (byte == EOF_BYTE) {
                    // Calculate expected CRC over the received payload
                    uint16_t calculated_crc = crc16_ccitt(rx_packet.payload, rx_packet.length);
                    
                    if (calculated_crc == rx_packet.crc) {
                        printf("[PARSER SUCCESS] Valid Packet Received! Length: %d\n", rx_packet.length);
                        // Protocol action can happen here (e.g., executing a command)
                    } else {
                        printf("[PARSER ERROR] CRC Mismatch! Expected: 0x%04X, Got: 0x%04X\n", calculated_crc, rx_packet.crc);
                    }
                } else {
                    printf("[PARSER ERROR] Missing EOF Byte!\n");
                }
                // Always reset back to look for next Start of Frame
                current_state = STATE_WAIT_SOF;
                break;
        }
    }
}
