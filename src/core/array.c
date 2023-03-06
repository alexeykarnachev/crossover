#include "../array.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_ARRAY_CAPACITY 1024

Array init_array(void) {
    int size = INITIAL_ARRAY_CAPACITY * sizeof(float);
    float* data = (float*)malloc(size);
    memset(data, 0, size);
    Array array = {
        .data = data, .length = 0, .capacity = INITIAL_ARRAY_CAPACITY};
    return array;
}

void destroy_array(Array* arr) {
    free(arr->data);
    arr->data = NULL;
    arr->length = 0;
    arr->capacity = INITIAL_ARRAY_CAPACITY;
}

static void resize_array(Array* arr) {
    int new_capacity = arr->capacity * 2;
    int new_size = new_capacity * sizeof(float);
    float* new_data = (float*)malloc(new_size);
    memset(new_data, 0, new_size);

    memcpy(new_data, arr->data, arr->capacity * sizeof(float));
    free(arr->data);

    arr->capacity = new_capacity;
    arr->data = new_data;
}

void array_push(Array* arr, float val) {
    if (arr->data == NULL || arr->capacity == 0) {
        fprintf(stderr, "ERROR: Can't push into the destroyed array\n");
        exit(1);
    }

    arr->data[arr->length++] = val;
    if (arr->length == arr->capacity) {
        resize_array(arr);
    }
}

float array_get(Array* arr, int idx) {
    if (idx >= arr->length || idx < 0) {
        fprintf(
            stderr,
            "ERROR: Can't get the value at index: %d from the array with "
            "length: %d\n",
            idx,
            arr->length
        );
        exit(1);
    }
    return arr->data[idx];
}

void array_set(Array* arr, int idx, float val) {
    if (idx >= arr->length || idx < 0) {
        fprintf(
            stderr,
            "ERROR: Can't set the value at index: %d from the array with "
            "length: %d\n",
            idx,
            arr->length
        );
        exit(1);
    }
    arr->data[idx] = val;
}

float array_peek(Array* arr) {
    if (arr->length <= 0) {
        fprintf(stderr, "ERROR: Can't peek from the empty array\n");
        exit(1);
    }
    return arr->data[arr->length - 1];
}

void array_remove_value(Array* arr, float remove_val) {
    int idx = -1;
    for (int i = 0; i < arr->length; ++i) {
        float this_val = array_get(arr, i);
        if (this_val != remove_val && idx != -1) {
            array_set(arr, idx, this_val);
            array_set(arr, i, remove_val);
            if (array_get(arr, idx + 1) == remove_val) {
                idx += 1;
            } else {
                idx = i;
            }
        } else if (this_val == remove_val && idx == -1) {
            idx = i;
        }
    }

    arr->length = idx != -1 ? idx : arr->length;
}
