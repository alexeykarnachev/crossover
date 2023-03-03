#include <assert.h>
#include <stdio.h>
#include "tests.h"
#include "../src/hashmap.h"

static void test_init_hashmap() {
    HashMap hm = init_hashmap();
    assert(hm.length == 0);
    assert(hm.capacity == 16);
    assert(hm.items != NULL);
    destroy_hashmap(&hm);
}

static void test_hashmap_put() {
    HashMap hm = init_hashmap();
    int value1 = 42, value2 = 24, value3 = 69;

    hashmap_put(&hm, "key1", &value1);
    hashmap_put(&hm, "key2", &value2);
    assert(hm.length == 2);
    assert(*(int*)hashmap_get(&hm, "key1") == 42);
    assert(*(int*)hashmap_get(&hm, "key2") == 24);

    hashmap_put(&hm, "key1", &value2);
    hashmap_put(&hm, "key2", &value1);
    assert(hm.length == 2);
    assert(*(int*)hashmap_get(&hm, "key1") == 24);
    assert(*(int*)hashmap_get(&hm, "key2") == 42);

    hashmap_put(&hm, "key3", &value3);
    assert(hm.length == 3);
    assert(*(int*)hashmap_get(&hm, "key1") == 24);
    assert(*(int*)hashmap_get(&hm, "key2") == 42);
    assert(*(int*)hashmap_get(&hm, "key3") == 69);

    destroy_hashmap(&hm);
}

static void test_hashmap_get() {
    HashMap hm = init_hashmap();
    int value1 = 42, value2 = 24;
    hashmap_put(&hm, "key1", &value1);
    hashmap_put(&hm, "key2", &value2);
    assert(*(int*)hashmap_get(&hm, "key1") == 42);
    assert(*(int*)hashmap_get(&hm, "key2") == 24);
    destroy_hashmap(&hm);
}

static void test_hashmap_try_get() {
    HashMap hm = init_hashmap();
    int value1 = 42, value2 = 24;
    hashmap_put(&hm, "key1", &value1);
    hashmap_put(&hm, "key2", &value2);
    assert(*(int*)hashmap_try_get(&hm, "key1") == 42);
    assert(*(int*)hashmap_try_get(&hm, "key2") == 24);
    assert(hashmap_try_get(&hm, "key3") == NULL);
    destroy_hashmap(&hm);
}

int test_hashmap(void) {
    test_init_hashmap();
    test_hashmap_put();
    test_hashmap_get();
    test_hashmap_try_get();
    printf("INFO: HashMap tests passed\n");
    return 0;
}
