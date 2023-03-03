#include <assert.h>
#include <stdio.h>
#include "tests.h"
#include "../src/ring_buffer.h"
#include <stdlib.h>

static RingBuffer test_init_ring_buffer(int capacity) {
    RingBuffer rb = init_ring_buffer(capacity);
    assert(rb.data != NULL);
    assert(rb.start == 0);
    assert(rb.end == 0);
    assert(rb.length == 0);
    assert(rb.capacity == capacity);

    return rb;
}

static RingBuffer* test_alloc_ring_buffer(int capacity) {
    RingBuffer* rb = alloc_ring_buffer(capacity);
    assert(rb->data != NULL);
    assert(rb->start == 0);
    assert(rb->end == 0);
    assert(rb->length == 0);
    assert(rb->capacity == capacity);

    return rb;
}

static void test_ring_buffer_push(RingBuffer* rbp) {
    ring_buffer_push(rbp, 1.0f);
    ring_buffer_push(rbp, 2.0f);
    ring_buffer_push(rbp, 3.0f);
    assert(rbp->length == 3);
    assert(rbp->data[0] == 1.0f);
    assert(rbp->data[1] == 2.0f);
    assert(rbp->data[2] == 3.0f);

    ring_buffer_push(rbp, 4.0f);
    ring_buffer_push(rbp, 5.0f);
    ring_buffer_push(rbp, 6.0f);
    assert(rbp->data[0] == 6.0f);
    assert(rbp->data[1] == 2.0f);
    assert(rbp->data[2] == 3.0f);
    assert(rbp->data[3] == 4.0f);
    assert(rbp->data[4] == 5.0f);

    float* linear_buffer = linearize_ring_buffer(rbp);
    assert(linear_buffer[0] == 2.0f);
    assert(linear_buffer[1] == 3.0f);
    assert(linear_buffer[2] == 4.0f);
    assert(linear_buffer[3] == 5.0f);
    assert(linear_buffer[4] == 6.0f);

    free(linear_buffer);
}

int test_ring_buffer(void) {
    RingBuffer rb = test_init_ring_buffer(5);
    test_ring_buffer_push(&rb);
    destroy_ring_buffer_data(&rb);

    RingBuffer* rbp = test_alloc_ring_buffer(5);
    test_ring_buffer_push(rbp);
    destroy_ring_buffer_data(rbp);
    free(rbp);

    printf("INFO: RingBuffer tests passed\n");
    return 0;
}
