#include "ring_buffer.h"

// Initialize head and tail pointers
void ring_buffer_init(ring_buffer_t *rb) {
    rb->head = 0;
    rb->tail = 0;
}

// Check if buffer is full
bool ring_buffer_is_full(const ring_buffer_t *rb) {
    return ((uint16_t)(rb->head + 1) % RING_BUFFER_SIZE) == rb->tail;
}

// Check if buffer is empty
bool ring_buffer_is_empty(const ring_buffer_t *rb) {
    return rb->head == rb->tail;
}

// Add a byte to the buffer (Called inside UART ISR)
bool ring_buffer_queue(ring_buffer_t *rb, uint8_t byte) {
    if (ring_buffer_is_full(rb)) {
        return false; // Buffer overflow! Data lost because parser is too slow.
    }
    
    rb->buffer[rb->head] = byte;
    rb->head = (rb->head + 1) % RING_BUFFER_SIZE;
    return true;
}

// Remove a byte from the buffer (Called in Main Loop)
bool ring_buffer_dequeue(ring_buffer_t *rb, uint8_t *byte) {
    if (ring_buffer_is_empty(rb)) {
        return false; // No data available
    }
    
    *byte = rb->buffer[rb->tail];
    rb->tail = (rb->tail + 1) % RING_BUFFER_SIZE;
    return true;
}
