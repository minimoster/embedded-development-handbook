#ifndef EMBEDDED_CORE_H
#define EMBEDDED_CORE_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
/* Contracts are tested on the host; this header is not an MMIO/RTOS port. */
bool make_mask(unsigned bit, uint32_t *out);
bool add_u32(uint32_t a, uint32_t b, uint32_t *out);
bool read_be16(const uint8_t *bytes, size_t n, uint16_t *out);
uint32_t elapsed_ticks(uint32_t now, uint32_t then);
/* Initialize ring with {0}; r must be non-NULL. Sequential capacity 7 only.
 * No ISR/thread/DMA concurrency guarantee. */
struct ring { uint8_t data[8]; unsigned head, tail; };
bool ring_push(struct ring *r, uint8_t value);
bool ring_pop(struct ring *r, uint8_t *out);
/* Stable-input debounce. Call often enough; one observed interval < 2^32 ticks. */
struct debounce { bool stable, candidate; uint32_t since; };
bool debounce_update(struct debounce *d, bool raw, uint32_t now, uint32_t threshold);
/* Toy wire format: length(0..16), payload, XOR(length,payload). Not security.
 * p must be initialized with {0}, data has n readable bytes, callback must not
 * reenter/mutate this parser; payload is borrowed only for that callback. */
#define FRAME_MAX 16u
struct parser { unsigned state, length, used; uint8_t checksum, body[FRAME_MAX]; size_t rejected; };
typedef void (*frame_fn)(void *ctx, const uint8_t *payload, size_t n);
void parser_feed(struct parser *p, const uint8_t *data, size_t n, frame_fn fn, void *ctx);
#endif
