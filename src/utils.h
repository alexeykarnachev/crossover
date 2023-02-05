#pragma once
#include "nfd.h"
#include <stdio.h>

char* read_bin_file(const char* restrict file_path, long* n_bytes);
char* read_cstr_file(
    const char* restrict file_path, const char* mode, long* n_bytes
);
nfdchar_t* open_file_path_via_nfd(
    const nfdchar_t* search_path,
    nfdfilteritem_t* filter_items,
    int n_filter_items
);
nfdchar_t* save_file_path_via_nfd(
    const nfdchar_t* search_path,
    nfdfilteritem_t* filter_items,
    int n_filter_items
);
void write_str_to_file(const char* str, FILE* fp, int allow_null);
void read_str_from_file(const char** str_p, FILE* fp, int allow_null);
