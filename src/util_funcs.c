#include "util_funcs.h"

int mem_equal(const void *arr1, const void *arr2, unsigned int len) {
    while (len--) {
        if (((unsigned char *) arr1)[len] != ((unsigned char *) arr2)[len]) {
            return 0;
        }
    }
    return 1;
}