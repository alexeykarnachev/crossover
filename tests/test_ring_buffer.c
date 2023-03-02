#include <assert.h>
#include <stdio.h>
#include "tests.h"
#include "../src/ring_buffer.h"
#include <stdlib.h>

static void test_ring_buffer_init() {
    RingBuffer rb = init_ring_buffer(5);
    assert(rb.data != NULL);
    assert(rb.start == 0);
    assert(rb.end == 0);
    assert(rb.length == 0);
    assert(rb.capacity == 5);
    destroy_ring_buffer(&rb);
}

static void test_ring_buffer_push() {
    RingBuffer rb = init_ring_buffer(5);
    ring_buffer_push(&rb, 1.0f);
    ring_buffer_push(&rb, 2.0f);
    ring_buffer_push(&rb, 3.0f);
    assert(rb.length == 3);
    assert(rb.data[0] == 1.0f);
    assert(rb.data[1] == 2.0f);
    assert(rb.data[2] == 3.0f);

    ring_buffer_push(&rb, 4.0f);
    ring_buffer_push(&rb, 5.0f);
    ring_buffer_push(&rb, 6.0f);
    assert(rb.data[0] == 6.0f);
    assert(rb.data[1] == 2.0f);
    assert(rb.data[2] == 3.0f);
    assert(rb.data[3] == 4.0f);
    assert(rb.data[4] == 5.0f);

    float* linear_buffer = linearize_ring_buffer(&rb);
    assert(linear_buffer[0] == 2.0f);
    assert(linear_buffer[1] == 3.0f);
    assert(linear_buffer[2] == 4.0f);
    assert(linear_buffer[3] == 5.0f);
    assert(linear_buffer[4] == 6.0f);

    destroy_ring_buffer(&rb);
}

static void test_ring_buffer_linearize() {
    RingBuffer rb = init_ring_buffer(5);
    ring_buffer_push(&rb, 1.0f);
    ring_buffer_push(&rb, 2.0f);
    ring_buffer_push(&rb, 3.0f);
    float* linear_buffer = linearize_ring_buffer(&rb);
    assert(linear_buffer[0] == 1.0f);
    assert(linear_buffer[1] == 2.0f);
    assert(linear_buffer[2] == 3.0f);
    free(linear_buffer);
    destroy_ring_buffer(&rb);
}

int test_ring_buffer(void) {
    test_ring_buffer_init();
    test_ring_buffer_push();
    test_ring_buffer_linearize();
    printf("INFO: RingBuffer tests passed\n");
    return 0;
}
