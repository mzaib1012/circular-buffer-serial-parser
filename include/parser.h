#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>
#include <stdbool.h>
#include "ring_buffer.h"

#define MAX_PAYLOAD_SIZE 64

// Control Characters
#define SOF_BYTE 0x02  // Start of Frame (STX)
#define EOF_BYTE 0x03  // End of Frame (ETX)

// Parser States
typedef enum {
    STATE_WAIT_SOF,
    STATE_GET_LENGTH,
    STATE_GET_PAYLOAD,
    STATE_GET_CRC_HIGH,
    STATE_GET_CRC_LOW,
    STATE_WAIT_EOF
} parser_state_t;

// Packet Structure
typedef struct {
    uint8_t payload[MAX_PAYLOAD_SIZE];
    uint8_t length;
    uint16_t crc;
} packet_t;

// Function Prototypes
void parser_init(void);
void parser_process(ring_buffer_t *rb);
uint16_t crc16_ccitt(const uint8_t *data, uint16_t length);

#endif // PARSER_H
