#ifndef EMBEDDED_COURSE_DEVICE_BOUNDARY_H
#define EMBEDDED_COURSE_DEVICE_BOUNDARY_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
struct device;
enum { DEVICE_OK = 0, DEVICE_ARGUMENT = 1, DEVICE_MEMORY = 2, DEVICE_EXHAUSTED = 3 };
/* out is writable; failure sets *out to null. limit must be nonzero. */
int device_create(uint32_t limit, struct device **out);
/* Emits 0..limit-1. Failure leaves *out unchanged; no concurrent calls. */
int device_read(struct device *owner, uint32_t *out);
/* Null is allowed. A nonnull handle must be a live handle from this library. */
void device_destroy(struct device *owner);
#ifdef __cplusplus
}
#endif
#endif
