#pragma once
#include "./asset/brain.h"
#include <stdint.h>

typedef enum AssetType { NULL_ASSET = 0, BRAIN_ASSET } AssetType;

typedef struct Asset {
    char file_path[MAX_PATH_LENGTH];

    AssetType type;
    struct {
        Brain brain;
    } a;
} Asset;

extern Asset ASSETS[MAX_N_ASSETS];
extern int N_ASSETS;

void reset_assets(void);
int load_asset(char* file_path, ResultMessage* res_msg);
Asset* get_asset(char* file_path);
