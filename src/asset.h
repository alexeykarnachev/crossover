#pragma once
#include "./asset/brain.h"

typedef enum AssetType { BRAIN_ASSET = 1 } AssetType;

typedef struct Asset {
    char file_path[MAX_PATH_LENGTH];

    AssetType type;
    struct {
        Brain brain;
    } a;
} Asset;

extern Asset ASSETS[MAX_N_ASSETS];
extern int N_ASSETS;

int add_asset(Asset asset);
