#pragma once

char* read_bin_file(const char* restrict file_path, long* n_bytes);
char* read_cstr_file(
    const char* restrict file_path, const char* mode, long* n_bytes
);
