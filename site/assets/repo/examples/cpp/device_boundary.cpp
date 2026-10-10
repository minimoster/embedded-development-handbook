#include "device_boundary.h"
#include <memory>
#include <new>
/* Fake host resource, not an actual sensor or driver. No throwing constructor. */
struct device { uint32_t limit; uint32_t next; };
int device_create(uint32_t limit, device **out) {
    if (!out) return DEVICE_ARGUMENT;
    *out = nullptr;
    if (!limit) return DEVICE_ARGUMENT;
    std::unique_ptr<device> fresh(new (std::nothrow) device{limit, 0});
    if (!fresh) return DEVICE_MEMORY;
    *out = fresh.release();
    return DEVICE_OK;
}
int device_read(device *owner, uint32_t *out) {
    if (!owner || !out) return DEVICE_ARGUMENT;
    if (owner->next == owner->limit) return DEVICE_EXHAUSTED;
    *out = owner->next++;
    return DEVICE_OK;
}
void device_destroy(device *owner) { delete owner; }
