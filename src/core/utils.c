#include "../utils.h"

#include "nfd.h"
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

FILE* open_file(
    const char* file_path, ResultMessage* res_msg, const char* mode
) {
    memset(res_msg, 0, sizeof(ResultMessage));

    if (file_path == NULL) {
        strcpy(res_msg->msg, "ERROR: Can't open the NULL file\n");
        return NULL;
    }

    FILE* fp = fopen(file_path, mode);
    if (!fp) {
        static char msg[MAX_RESULT_MESSAGE_LENGTH + 64];
        sprintf(msg, "ERROR: Can't open the file: %s\n", file_path);
        strncpy(res_msg->msg, msg, MAX_RESULT_MESSAGE_LENGTH);
        return NULL;
    }

    res_msg->flag = SUCCESS_RESULT;
    return fp;
}

char* read_cstr_file(
    const char* restrict file_path, const char* mode, long* n_bytes
) {
    FILE* file = NULL;
    char* content = NULL;

    file = fopen(file_path, mode);
    if (file == NULL)
        goto fail;
    if (fseek(file, 0, SEEK_END) < 0)
        goto fail;

    long size = ftell(file);
    if (size < 0)
        goto fail;
    if (fseek(file, 0, SEEK_SET) < 0)
        goto fail;

    content = malloc(size + 1);
    if (content == NULL)
        goto fail;

    long read_size = fread(content, 1, size, file);
    if (ferror(file))
        goto fail;

    content[size] = '\0';
    if (file) {
        fclose(file);
        errno = 0;
    }

    if (n_bytes != NULL) {
        *n_bytes = size;
    }
    return content;

fail:
    if (file) {
        int e = errno;
        fclose(file);
        errno = e;
    }
    if (content) {
        free(content);
    }
    return NULL;
}

nfdchar_t* open_nfd(
    const nfdchar_t* search_path,
    nfdfilteritem_t* filter_items,
    int n_filter_items
) {
    NFD_Init();
    nfdchar_t* file_path;
    nfdresult_t result = NFD_OpenDialog(
        &file_path, filter_items, n_filter_items, search_path
    );

    if (result == NFD_OKAY) {
        NFD_Quit();
    } else if (result == NFD_CANCEL) {
        file_path = NULL;
    } else {
        fprintf(stderr, "ERROR: %s\n", NFD_GetError());
        exit(1);
    }

    return file_path;
}

nfdchar_t* save_nfd(
    const nfdchar_t* search_path,
    nfdfilteritem_t* filter_items,
    int n_filter_items
) {
    NFD_Init();
    nfdchar_t* file_path;
    nfdresult_t result = NFD_SaveDialogN(
        &file_path, filter_items, n_filter_items, search_path, NULL
    );
    if (result == NFD_OKAY) {
        NFD_Quit();
    } else if (result == NFD_CANCEL) {
        file_path = NULL;
    } else {
        fprintf(stderr, "ERROR: %s\n", NFD_GetError());
        exit(1);
    }

    return file_path;
}

int write_str_to_file(const char* str, FILE* fp, int allow_null) {
    if (str == NULL && !allow_null) {
        fprintf(
            stderr,
            "ERROR: Can't write NULL string to the file. Set allow_null = "
            "1 if you still want to do this\n"
        );
        exit(1);
    }

    int str_len = 0;
    int n_bytes = 0;
    if (str == NULL) {
        n_bytes += fwrite(&str_len, sizeof(int), 1, fp);
    } else {
        str_len = strlen(str) + 1;
        n_bytes += fwrite(&str_len, sizeof(int), 1, fp);
        n_bytes += fwrite(str, sizeof(char), str_len, fp);
    }

    return n_bytes;
}

int read_str_from_file(char** str_p, FILE* fp, int allow_null) {
    uint32_t str_len;
    int n_bytes = 0;
    n_bytes += fread(&str_len, sizeof(uint32_t), 1, fp);
    if (str_len > 0) {
        char* buffer = (char*)malloc(str_len + 1);
        n_bytes += fread(buffer, sizeof(char), str_len, fp);
        buffer[str_len] = '\0';
        *str_p = buffer;
    } else if (allow_null) {
        *str_p = NULL;
    } else {
        fprintf(
            stderr,
            "ERROR: Can't read 0 length string from the file. Set "
            "allow_null = 1 if you still want to do this\n"
        );
        exit(1);
    }

    return n_bytes;
}

uint64_t get_bytes_hash(const char* bytes, int n_bytes) {
    uint64_t p1 = 7;
    uint64_t p2 = 31;

    uint64_t hash = p1;
    for (const char* p = bytes; n_bytes != 0; n_bytes--) {
        hash = hash * p2 + *p;
    }

    return hash;
}

float frand01(void) {
    srand(time(NULL));
    return (float)rand() / RAND_MAX;
}

static void merge(
    float arr[],
    int idx[],
    float left[],
    int left_idx[],
    int left_size,
    float right[],
    int right_idx[],
    int right_size,
    int descending
) {
    int i = 0, j = 0, k = 0;

    while (i < left_size && j < right_size) {
        if ((descending && left[i] >= right[j])
            || (!descending && left[i] <= right[j])) {
            arr[k] = left[i];
            idx[k] = left_idx[i];
            i++;
        } else {
            arr[k] = right[j];
            idx[k] = right_idx[j];
            j++;
        }
        k++;
    }

    while (i < left_size) {
        arr[k] = left[i];
        idx[k] = left_idx[i];
        i++;
        k++;
    }

    while (j < right_size) {
        arr[k] = right[j];
        idx[k] = right_idx[j];
        j++;
        k++;
    }
}

void sort(float arr[], int idx[], int length, int descending) {
    if (length < 2) {
        return;
    }

    int mid = length / 2;
    float left[mid], right[length - mid];
    int left_idx[mid], right_idx[length - mid];

    for (int i = 0; i < mid; i++) {
        left[i] = arr[i];
        left_idx[i] = idx[i];
    }

    for (int i = mid; i < length; i++) {
        right[i - mid] = arr[i];
        right_idx[i - mid] = idx[i];
    }

    sort(left, left_idx, mid, descending);
    sort(right, right_idx, length - mid, descending);
    merge(
        arr,
        idx,
        left,
        left_idx,
        mid,
        right,
        right_idx,
        length - mid,
        descending
    );
}

void argsort(float arr[], int idx[], int length, int descending) {
    for (int i = 0; i < length; i++) {
        idx[i] = i;
    }

    sort(arr, idx, length, descending);
}
