#pragma once

typedef struct Array {
    float* data;
    int length;
    int capacity;
} Array;

Array init_array(void);
void destroy_array(Array* arr);
void empty_array(Array* arr);
void append_array(Array* dest, Array* src);
void array_push(Array* arr, float val);
float array_get(Array* arr, int idx);
void array_set(Array* arr, int idx, float val);
float array_peek(Array* arr);

void array_remove_value(
    Array* arr, float remove_val, int fail_if_not_present
);
