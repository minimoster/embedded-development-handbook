#include "../examples/cpp/device_boundary.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
int main(void) {
    struct device *owner = NULL;
    assert(device_create(3, NULL) == DEVICE_ARGUMENT);
    assert(device_create(0, &owner) == DEVICE_ARGUMENT && owner == NULL);
    assert(device_create(3, &owner) == DEVICE_OK && owner != NULL);
    uint32_t value = 99;
    assert(device_read(NULL, &value) == DEVICE_ARGUMENT && value == 99);
    assert(device_read(owner, NULL) == DEVICE_ARGUMENT);
    for (uint32_t i = 0; i < 3; ++i) {
        assert(device_read(owner, &value) == DEVICE_OK && value == i);
    }
    value = 99;
    assert(device_read(owner, &value) == DEVICE_EXHAUSTED && value == 99);
    device_destroy(owner); owner = NULL;
    device_destroy(owner);
    assert(device_create(UINT32_MAX, &owner) == DEVICE_OK);
    assert(device_read(owner, &value) == DEVICE_OK && value == 0);
    device_destroy(owner);
    puts("C/C++ ABI host: PASS (real C caller, C++ implementation, opaque ownership/error contract)");
    return 0;
}
