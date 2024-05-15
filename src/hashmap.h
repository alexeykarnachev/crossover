#pragma once
#include "const.h"

typedef struct HashMapItem {
    char key[MAX_HASHMAP_KEY_LENGTH];
    void *value;
} HashMapItem;

typedef struct HashMap {
    HashMapItem *items;
    int length;
    int capacity;
} HashMap;

HashMap init_hashmap(void);
void destroy_hashmap(HashMap *hashmap);
void hashmap_put(HashMap *hashmap, char *key, void *value);
void *hashmap_get(HashMap *hashmap, char *key);
void *hashmap_try_get(HashMap *hashmap, char *key);
void print_hashmap(HashMap *hashmap);
