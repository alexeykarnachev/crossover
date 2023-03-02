#include "../hashmap.h"

#include "../const.h"
#include "../utils.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_HASHMAP_CAPACITY 16
#define LOAD_FACTOR 0.75

HashMap init_hashmap(void) {
    int size = INITIAL_HASHMAP_CAPACITY * sizeof(HashMapItem);
    HashMapItem* items = (HashMapItem*)malloc(size);
    memset(items, 0, size);
    HashMap hashmap = {
        .items = items, .length = 0, .capacity = INITIAL_HASHMAP_CAPACITY};
    return hashmap;
}

void destroy_hashmap(HashMap* hashmap) {
    free(hashmap->items);
    hashmap->items = NULL;
    hashmap->length = 0;
    hashmap->capacity = 0;
}

void destroy_hashmap_and_values(HashMap* hashmap) {
    for (int i = 0; i < hashmap->length; ++i) {
        HashMapItem item = hashmap->items[i];
        if (item.key[0] != '\0') {
            free(item.value);
        }
    }

    destroy_hashmap(hashmap);
}

static uint64_t get_key_hash(char* key) {
    int key_length = strlen(key);
    if (key_length > MAX_HASHMAP_KEY_LENGTH) {
        fprintf(
            stderr,
            "ERROR: Hashmap key can't be longer than %d characters, but "
            "your key: %s has length %d\n",
            MAX_HASHMAP_KEY_LENGTH,
            key,
            key_length
        );
        exit(1);
    }
    uint64_t hash = get_bytes_hash(key, key_length);
    return hash;
}

static void resize_hashmap(HashMap* hashmap) {
    int old_length = hashmap->length;
    int old_capacity = hashmap->capacity;
    int new_capacity = old_capacity * 2;
    int new_size = new_capacity * sizeof(HashMapItem);

    HashMapItem* old_items = hashmap->items;
    HashMapItem* new_items = (HashMapItem*)malloc(new_size);
    memset(new_items, 0, new_size);

    hashmap->items = new_items;
    hashmap->capacity = new_capacity;
    hashmap->length = 0;

    for (int old_idx = 0; old_idx < old_capacity; ++old_idx) {
        HashMapItem item = old_items[old_idx];
        if (item.key[0] != '\0') {
            hashmap_put(hashmap, item.key, item.value);
        }
    }

    if (hashmap->length != old_length) {
        fprintf(
            stderr,
            "ERROR: Something is wrong during the hasmap resize. After "
            "putting back all the items in the new hashmap, the new "
            "length is: %d, but the old length was: %d\n",
            hashmap->length,
            old_length
        );
    }

    free(old_items);
}

void hashmap_put(HashMap* hashmap, char* key, void* value) {
    uint64_t hash = get_key_hash(key);
    int idx = hash % hashmap->capacity;
    while (hashmap->items[idx].key[0] != '\0') {
        if (strcmp(hashmap->items[idx].key, key) != 0) {
            idx = (idx + 1) % hashmap->capacity;
        } else {
            hashmap->length -= 1;
            break;
        }
    }
    HashMapItem item = {0};
    strcpy(item.key, key);
    item.value = value;
    hashmap->items[idx] = item;
    hashmap->length += 1;

    float load_factor = (float)hashmap->length / (float)hashmap->capacity;
    if (load_factor >= LOAD_FACTOR) {
        resize_hashmap(hashmap);
    }
}

void* hashmap_get(HashMap* hashmap, char* key) {
    void* item = hashmap_try_get(hashmap, key);
    if (item == NULL) {
        fprintf(stderr, "ERROR: No such key in hashmap: %s\n", key);
        exit(1);
    }
    return item;
}

void* hashmap_try_get(HashMap* hashmap, char* key) {
    uint64_t hash = get_key_hash(key);
    int idx = hash % hashmap->capacity;
    char* candidate_key = hashmap->items[idx].key;
    int items_checked = 0;
    int hashmap_length = hashmap->length;
    int hashmap_capacity = hashmap->capacity;

    do {
        if (candidate_key[0] == '\0' || items_checked == hashmap_length) {
            return NULL;
        } else if (strcmp(candidate_key, key) == 0) {
            return hashmap->items[idx].value;
        }
        idx = (idx + 1) % hashmap_capacity;
        items_checked++;
    } while (1);
}

void print_hashmap(HashMap* hashmap) {
    printf("{\n");
    for (int i = 0; i < hashmap->capacity; ++i) {
        HashMapItem item = hashmap->items[i];
        if (item.key[0] != '\0') {
            printf("    %s: %p,\n", item.key, item.value);
        }
    }
    printf("}\n");
}
