#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>
#include <stdbool.h>

// Buffer size must be a power of 2 for fast bitwise modulo operations
#define RING_BUFFER_SIZE 128

typedef struct {
    uint8_t buffer[RING_BUFFER_SIZE];
    volatile uint16_t head;  // Written by ISR (Interrupt)
    volatile uint16_t tail;  // Read by Main Loop
} ring_buffer_t;

// Function prototypes
void ring_buffer_init(ring_buffer_t *rb);
bool ring_buffer_is_full(const ring_buffer_t *rb);
bool ring_buffer_is_empty(const ring_buffer_t *rb);
bool ring_buffer_queue(ring_buffer_t *rb, uint8_t byte);
bool ring_buffer_dequeue(ring_buffer_t *rb, uint8_t *byte);

#endif // RING_BUFFER_H
