#include "../utils.h"

#include "../math.h"
#include <errno.h>
#include <float.h>
#include <math.h>
#include <regex.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <sys/time.h>
#endif

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

void write_str_to_file(const char* str, FILE* fp, int allow_null) {
    if (str == NULL && !allow_null) {
        fprintf(
            stderr,
            "ERROR: Can't write NULL string to the file. Set allow_null = "
            "1 if you still want to do this\n"
        );
        exit(1);
    }

    int str_len = 0;
    if (str == NULL) {
        fwrite(&str_len, sizeof(int), 1, fp);
    } else {
        str_len = strlen(str) + 1;
        fwrite(&str_len, sizeof(int), 1, fp);
        fwrite(str, sizeof(char), str_len, fp);
    }
}

void read_str_from_file(char** str_p, FILE* fp, int allow_null) {
    uint32_t str_len;
    fread(&str_len, sizeof(uint32_t), 1, fp);
    if (str_len > 0) {
        char* buffer = (char*)malloc(str_len + 1);
        fread(buffer, sizeof(char), str_len, fp);
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
}

char* increment_file_name_version(char* file_name) {
    regex_t regex;
    regmatch_t match[2];

    if (regcomp(&regex, "\\.v([0-9]+)\\.", REG_EXTENDED) != 0) {
        fprintf(stderr, "ERROR: Can't compile regex\n");
        exit(1);
    }

    char* new_file_name = NULL;
    char* version_str = NULL;
    int new_file_name_length = 0;
    if (regexec(&regex, file_name, 2, match, 0) == 0) {
        version_str = strndup(
            &file_name[match[1].rm_so], match[1].rm_eo - match[1].rm_so
        );
        int version = atoi(version_str);
        version += 1;
        new_file_name_length = snprintf(
            NULL,
            0,
            "%.*s.v%d%s",
            (int)match[1].rm_so,
            file_name,
            version,
            &file_name[match[0].rm_eo]
        );
        new_file_name = malloc(new_file_name_length + 1);
        sprintf(
            new_file_name,
            "%.*s.v%d.%s",
            (int)match[0].rm_so,
            file_name,
            version,
            &file_name[match[0].rm_eo]
        );
    } else {
        const char* ext = strrchr(file_name, '.');
        if (ext == NULL) {
            new_file_name_length = snprintf(NULL, 0, "%s.v0", file_name);
            new_file_name = malloc(new_file_name_length + 1);
            sprintf(new_file_name, "%s.v0", file_name);
        } else {
            new_file_name_length = snprintf(
                NULL,
                0,
                "%.*s.v0%s",
                (int)(ext - file_name),
                file_name,
                ext
            );
            new_file_name = malloc(new_file_name_length + 1);
            sprintf(
                new_file_name,
                "%.*s.v0%s",
                (int)(ext - file_name),
                file_name,
                ext
            );
        }
    }

    regfree(&regex);
    free(version_str);

    return new_file_name;
}

uint64_t get_bytes_hash(const char* bytes, int n_bytes) {
    uint64_t p1 = 7;
    uint64_t p2 = 31;

    uint64_t hash = p1;
    for (const char* p = bytes; n_bytes != 0; n_bytes--, p++) {
        hash = hash * p2 + *p;
    }

    return hash;
}

float frand01(void) {
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

void sort(float arr[], int idx[], int n, int descending) {
    if (n < 2) {
        return;
    }

    int mid = n / 2;
    float left[mid], right[n - mid];
    int left_idx[mid], right_idx[n - mid];

    for (int i = 0; i < mid; i++) {
        left[i] = arr[i];
        left_idx[i] = idx[i];
    }

    for (int i = mid; i < n; i++) {
        right[i - mid] = arr[i];
        right_idx[i - mid] = idx[i];
    }

    sort(left, left_idx, mid, descending);
    sort(right, right_idx, n - mid, descending);
    merge(
        arr,
        idx,
        left,
        left_idx,
        mid,
        right,
        right_idx,
        n - mid,
        descending
    );
}

void argsort(float arr[], int idx[], int n, int descending) {
    for (int i = 0; i < n; i++) {
        idx[i] = i;
    }

    sort(arr, idx, n, descending);
}

int choose_idx(int n) {
    float r = frand01();
    return (int)(r * n);
}

int argmax(float* vals, int n) {
    float max_val = -FLT_MAX;
    int max_val_idx = 0;
    for (int i = 0; i < n; ++i) {
        float val = *vals++;
        if (val > max_val) {
            max_val_idx = i;
            max_val = val;
        }
    }

    return max_val_idx;
}

float sigmoid(float x) {
    return 1.0 / (1.0 + exp(-x));
}

void softmax(float* x, int n) {
    float max_x = x[0];
    for (int i = 1; i < n; i++) {
        if (x[i] > max_x) {
            max_x = x[i];
        }
    }

    float sum_exp_x = 0.0;
    for (int i = 0; i < n; i++) {
        sum_exp_x += exp(x[i] - max_x);
    }

    for (int i = 0; i < n; i++) {
        x[i] = exp(x[i] - max_x) / sum_exp_x;
    }
}

int sample_binary(float weight, float temperature) {
    temperature = max(temperature, EPS);
    float score = sigmoid(weight / temperature);
    float rand = frand01();
    int res = rand < score ? 1 : 0;
    return res;
}

int sample_multinomial(float* weights, int n, float temperature) {
    temperature = max(temperature, EPS);
    static int* _idx = NULL;
    static float* _weights = NULL;
    static int _n = 0;
    if (_n < n) {
        _n = n;
        if (_idx != NULL) {
            free(_idx);
        }
        _idx = malloc(_n * sizeof(int));
        _weights = malloc(_n * sizeof(float));
    }
    memcpy(_weights, weights, n * sizeof(float));
    for (int i = 0; i < n; ++i) {
        _weights[i] /= temperature;
    }
    softmax(_weights, n);
    argsort(_weights, _idx, n, 0);

    float rnd = frand01();
    float cdf = 0.0;
    for (int i = 0; i < n; ++i) {
        cdf += _weights[i];
        if (rnd <= cdf || i == n - 1) {
            return _idx[i];
        }
    }

    fprintf(
        stderr, "ERROR: Unreachable in `sample_multinomial`. It's a bug\n"
    );
    exit(1);
}

void shuffle(int arr[], int n) {
    int i, j, tmp;
    for (i = n - 1; i > 0; i--) {
        j = rand() % (i + 1);
        tmp = arr[i];
        arr[i] = arr[j];
        arr[j] = tmp;
    }
}

void swap_ptrs(void** p0, void** p1) {
    void* tmp = *p0;
    *p0 = *p1;
    *p1 = tmp;
}

double get_curr_time(void) {
    double seconds;
#ifdef _WIN32
    FILETIME ft;
    ULARGE_INTEGER uli;

    GetSystemTimeAsFileTime(&ft);
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    seconds = uli.QuadPart / 10000000.0;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    seconds = tv.tv_sec + tv.tv_usec / 1000000.0;
#endif
    return seconds;
}
