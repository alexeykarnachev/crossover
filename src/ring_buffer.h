#pragma once

typedef struct RingBuffer {
    float* data;
    int start;
    int end;
    int length;
    int capacity;
} RingBuffer;

RingBuffer init_ring_buffer(int capacity);
RingBuffer* alloc_ring_buffer(int capacity);
float* linearize_ring_buffer(RingBuffer* ring_buffer);
void destroy_ring_buffer_data(RingBuffer* ring_buffer);
void ring_buffer_push(RingBuffer* ring_buffer, float val);
