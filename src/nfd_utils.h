#pragma once
#include "nfd.h"

nfdchar_t* open_nfd(
    const nfdchar_t* search_path,
    nfdfilteritem_t* filter_items,
    int n_filter_items
);

nfdchar_t* save_nfd(
    const nfdchar_t* search_path,
    nfdfilteritem_t* filter_items,
    int n_filter_items
);
