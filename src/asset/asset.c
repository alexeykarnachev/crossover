#include "../asset.h"

#include "../math.h"
#include "../utils.h"
#include "brain.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

Asset ASSETS[MAX_N_ASSETS] = {0};
int N_ASSETS = 0;

static AssetType get_asset_type(char* file_path) {
    if (file_path == NULL) {
        return NULL_ASSET;
    }

    int file_path_len = strlen(file_path);
    char* p = strrchr(file_path, '.');
    if (p == NULL) {
        return NULL_ASSET;
    } else if (strcmp(p + 1, "xbrain") == 0) {
        return BRAIN_ASSET;
    } else {
        return NULL_ASSET;
    }
}

int load_asset(char* file_path, ResultMessage* res_msg) {
    res_msg->flag = FAIL_RESULT;
    if (file_path == NULL) {
        sprintf(
            res_msg->msg,
            "ERROR: Can't load the asset from the NULL file path\n"
        );
        return 0;
    }

    AssetType type = get_asset_type(file_path);
    if (type == NULL_ASSET) {
        sprintf(
            res_msg->msg,
            "ERROR: Unable to load asset due to undefined (NULL_ASSET) "
            "type. Please ensure that the asset has a defined type when "
            "calling the `load_asset` function."
        );
        return 0;
    }

    if (N_ASSETS == MAX_N_ASSETS) {
        sprintf(
            res_msg->msg,
            "ERROR: Can't load more assets (maximum is reached: %d)\n",
            MAX_N_ASSETS
        );
        return 0;
    }

    int file_path_len = strlen(file_path);
    uint64_t hash = get_bytes_hash(file_path, file_path_len);
    Asset asset;
    strcpy(asset.file_path, file_path);

    int n_bytes = file_path_len + sizeof(type);
    switch (type) {
        case BRAIN_ASSET: {
            Brain brain = init_empty_brain();
            n_bytes += load_brain(file_path, &brain, res_msg);
            if (res_msg->flag != SUCCESS_RESULT) {
                return 0;
            }
            asset.type = type;
            asset.a.brain = brain;
            break;
        }
        default: {
            sprintf(
                res_msg->msg,
                "ERROR: Can't load asset with type id: %d. It's probably "
                "not implemented in the function `load_asset`\n",
                type
            );
            return 0;
        }
    }

    int idx = hash % MAX_N_ASSETS;
    while (ASSETS[idx].type != 0
           && strcmp(ASSETS[idx].file_path, file_path) != 0) {
        idx = (idx + 1) % MAX_N_ASSETS;
    }

    if (strcmp(ASSETS[idx].file_path, file_path) != 0) {
        N_ASSETS += 1;
        ASSETS[idx] = asset;
    }

    return n_bytes;
}

Asset* get_asset(char* file_path) {
    if (file_path == NULL) {
        return NULL;
    }

    int file_path_len = strlen(file_path);
    if (file_path_len == 0) {
        return NULL;
    }

    uint64_t hash = get_bytes_hash(file_path, file_path_len);
    int start_idx = hash % MAX_N_ASSETS;
    int idx = start_idx;

    do {
        if (strcmp(ASSETS[idx].file_path, file_path) == 0) {
            return &ASSETS[idx];
        }
        idx += 1;
    } while (ASSETS[idx].type != 0 && idx != start_idx);

    return NULL;
}

void reset_assets(void) {
    memset(ASSETS, 0, sizeof(ASSETS));
    N_ASSETS = 0;
}
