#include <assert.h>
#include <stdio.h>
#include "tests.h"
#include "../src/array.h"


int test_array(void) {
    // Test init_array()
    Array arr = init_array();
    int initial_capacity = arr.capacity;
    assert(arr.length == 0);
    assert(arr.capacity == initial_capacity);

    // Test array_push() and array_get()
    array_push(&arr, 1.0);
    array_push(&arr, 2.0);
    array_push(&arr, 3.0);
    assert(array_get(&arr, 0) == 1.0);
    assert(array_get(&arr, 1) == 2.0);
    assert(array_get(&arr, 2) == 3.0);

    // Test array_set()
    array_set(&arr, 1, 4.0);
    assert(array_get(&arr, 1) == 4.0);

    // Test array_peek()
    assert(array_peek(&arr) == 3.0);

    // Test array_remove_value()
    array_remove_value(&arr, 2.0, 0);
    assert(array_get(&arr, 0) == 1.0);
    assert(array_get(&arr, 1) == 4.0);
    assert(array_get(&arr, 2) == 3.0);
    assert(arr.length == 3);
    array_push(&arr, 4.0);
    array_push(&arr, 4.0);
    array_push(&arr, 4.0);
    array_push(&arr, 3.0);
    assert(arr.length == 7);
    array_remove_value(&arr, 4.0, 1);
    assert(array_get(&arr, 0) == 1.0);
    assert(array_get(&arr, 1) == 3.0);
    assert(array_get(&arr, 2) == 3.0);
    assert(arr.length == 3);

    // Test destroy_array()
    destroy_array(&arr);
    assert(arr.data == NULL);
    assert(arr.length == 0);
    assert(arr.capacity == initial_capacity);

    printf("INFO: Array tests passed\n");
    return 0;
}
