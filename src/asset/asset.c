#include "../asset.h"

#include "../math.h"
#include "../utils.h"
#include "brain.h"
#include <stdlib.h>

Asset ASSETS[MAX_N_ASSETS] = {0};
int N_ASSETS = 0;

int add_asset(Asset asset) {
    for (int i = 0; i < MAX_N_ASSETS; ++i) {
        if (ASSETS[i].type == 0) {
            ASSETS[i] = asset;
            N_ASSETS = max(N_ASSETS, i + 1);
            return i;
        }
    }

    fprintf(stderr, "ERROR: Can't add more assets\n");
    exit(1);
}
