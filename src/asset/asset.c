#include "../asset.h"

#include "../math.h"
#include "../utils.h"
#include "brain.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

Asset ASSETS[MAX_N_ASSETS] = {0};
int N_ASSETS = 0;

int load_asset(char* file_path, AssetType type) {
    if (file_path == NULL) {
        return 0;
    }

    if (N_ASSETS == MAX_N_ASSETS) {
        fprintf(stderr, "ERROR: Can't add more assets\n");
        exit(1);
    }

    int file_path_len = strlen(file_path);
    uint64_t hash = get_bytes_hash(file_path, file_path_len);
    Asset asset;
    strcpy(asset.file_path, file_path);

    int n_bytes = file_path_len + sizeof(type);
    ResultMessage res_msg;
    switch (type) {
        case BRAIN_ASSET: {
            Brain brain = init_empty_brain();
            n_bytes += load_brain(file_path, &brain, &res_msg);
            if (res_msg.flag != SUCCESS_RESULT) {
                fprintf(stderr, "%s", res_msg.msg);
                exit(1);
            }
            asset.type = type;
            asset.a.brain = brain;
            break;
        }
        default: {
            fprintf(
                stderr, "ERROR: Can't load asset with type id: %d\n", type
            );
            exit(1);
        }
    }

    int idx = hash % MAX_N_ASSETS;
    while (ASSETS[idx].type != 0
           && strcmp(ASSETS[idx].file_path, file_path) != 0) {
        idx = (idx + 1) % MAX_N_ASSETS;
    }

    ASSETS[idx] = asset;
    N_ASSETS += 1;
    return n_bytes;
}

void reset_assets(void) {
    memset(ASSETS, 0, sizeof(ASSETS));
    N_ASSETS = 0;
}
