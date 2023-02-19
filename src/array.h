#pragma once

typedef struct Array {
    float* data;
    int length;
    int capacity;
} Array;

Array init_array(void);
void destroy_array(Array* arr);
void array_push(Array* arr, float val);
float array_get(Array* arr, int idx);
float array_peek(Array* arr);
