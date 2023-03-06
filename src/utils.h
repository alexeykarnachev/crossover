#pragma once
#include "array.h"
#include "const.h"
#include <stdint.h>
#include <stdio.h>

typedef enum ResultFlag {
    UNKNOWN_RESULT = -1,
    FAIL_RESULT = 0,
    SUCCESS_RESULT = 1
} ResultFlag;

typedef struct ResultMessage {
    char msg[MAX_RESULT_MESSAGE_LENGTH];
    ResultFlag flag;
} ResultMessage;

FILE* open_file(
    const char* file_path, ResultMessage* res_msg, const char* mode
);
char* read_cstr_file(
    const char* restrict file_path, const char* mode, long* n_bytes
);
void read_str_from_file(char** str_p, FILE* fp, int allow_null);
void read_array_from_file(Array* array, FILE* fp);
void write_str_to_file(const char* str, FILE* fp, int allow_null);
void write_array_to_file(Array* array, FILE* fp);
uint64_t get_bytes_hash(const char* bytes, int n_bytes);
float frand01(void);

void argsort(float arr[], int idx[], int n, int descending);
void sort(float arr[], int idx[], int n, int descending);

int choose_idx(int n);
int argmax(float* vals, int n);
void softmax(float* x, int n);
float sigmoid(float x);
int sample_binary(float weight);
int sample_multinomial(float* weights, int n);
void shuffle(int arr[], int n);
void swap(void** p0, void** p1);

double get_current_time(void);
