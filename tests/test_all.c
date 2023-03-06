#include "tests.h"
#include <stdio.h>

void main(void) {
    printf("\nINFO: Testing...\n");
    
    test_array();
    test_hashmap();
    test_ring_buffer();
}
