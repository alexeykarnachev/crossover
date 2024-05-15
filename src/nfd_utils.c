#include "nfd_utils.h"

#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

nfdchar_t *open_nfd(
    const nfdchar_t *search_path, nfdfilteritem_t *filter_items, int n_filter_items
) {
    NFD_Init();
    nfdchar_t *file_path;
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

nfdchar_t *save_nfd(
    const nfdchar_t *search_path, nfdfilteritem_t *filter_items, int n_filter_items
) {
    NFD_Init();
    nfdchar_t *file_path;
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
