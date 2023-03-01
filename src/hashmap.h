#pragma once
#include "const.h"

typedef struct HashMapItem {
    char key[MAX_HASHMAP_KEY_LENGTH];
    float value;
} HashMapItem;

typedef struct HashMap {
    HashMapItem* items;
    int length;
    int capacity;
} HashMap;

HashMap init_hashmap(void);
void destroy_hashmap(HashMap* hashmap);
void hashmap_put(HashMap* hashmap, char* key, float val);
float hashmap_get(HashMap* hashmap, char* key);
