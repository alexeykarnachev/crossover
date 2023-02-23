#pragma once
#include "const.h"
#include "nfd.h"
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
nfdchar_t* open_nfd(
    const nfdchar_t* search_path,
    nfdfilteritem_t* filter_items,
    int n_filter_items
);
nfdchar_t* save_nfd(
    const nfdchar_t* search_path,
    nfdfilteritem_t* filter_items,
    int n_filter_items
);
int write_str_to_file(const char* str, FILE* fp, int allow_null);
int read_str_from_file(char** str_p, FILE* fp, int allow_null);
uint64_t get_bytes_hash(const char* bytes, int n_bytes);
float frand01(void);

void argsort(float arr[], int idx[], int length, int descending);
void sort(float arr[], int idx[], int length, int descending);

int choose_idx(int to);
void shuffle(int arr[], int n);
