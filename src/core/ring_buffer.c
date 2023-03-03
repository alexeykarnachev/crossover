#include "../ring_buffer.h"

#include <stdlib.h>

RingBuffer init_ring_buffer(int capacity) {
    RingBuffer rb;
    rb.data = (float*)malloc(sizeof(float) * capacity);
    rb.start = 0;
    rb.end = 0;
    rb.length = 0;
    rb.capacity = capacity;
    return rb;
}

RingBuffer* alloc_ring_buffer(int capacity) {
    RingBuffer rb = init_ring_buffer(capacity);
    RingBuffer* rbp = (RingBuffer*)malloc(sizeof(RingBuffer));
    *rbp = rb;
    return rbp;
}

float* linearize_ring_buffer(RingBuffer* ring_buffer) {
    float* linear_buffer = (float*)malloc(
        sizeof(float) * ring_buffer->length
    );

    int i;
    int j = 0;
    for (i = ring_buffer->start; j < ring_buffer->length;
         i = (i + 1) % ring_buffer->capacity) {
        linear_buffer[j++] = ring_buffer->data[i];
    }

    return linear_buffer;
}

void destroy_ring_buffer_data(RingBuffer* ring_buffer) {
    free(ring_buffer->data);
    ring_buffer->data = NULL;
    ring_buffer->start = 0;
    ring_buffer->end = 0;
    ring_buffer->length = 0;
    ring_buffer->capacity = 0;
}

void ring_buffer_push(RingBuffer* ring_buffer, float val) {
    if (ring_buffer->length == ring_buffer->capacity) {
        ring_buffer->data[ring_buffer->start] = val;
        ring_buffer->start = (ring_buffer->start + 1)
                             % ring_buffer->capacity;
        ring_buffer->end = (ring_buffer->end + 1) % ring_buffer->capacity;
    } else {
        ring_buffer->data[ring_buffer->end] = val;
        ring_buffer->end = (ring_buffer->end + 1) % ring_buffer->capacity;
        ring_buffer->length++;
    }
}
