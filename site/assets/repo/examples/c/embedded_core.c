#include "embedded_core.h"
#include <limits.h>
bool make_mask(unsigned bit, uint32_t *out) {
    if (bit >= 32 || !out) return false;
    *out = UINT32_C(1) << bit; return true;
}
bool add_u32(uint32_t a, uint32_t b, uint32_t *out) {
    if (!out || b > UINT32_MAX-a) return false;
    *out = a+b; return true;
}
bool read_be16(const uint8_t *bytes, size_t n, uint16_t *out) {
    if (!bytes || !out || n < 2) return false;
    *out = (uint16_t)(((uint16_t)bytes[0]<<8) | bytes[1]); return true;
}
uint32_t elapsed_ticks(uint32_t now, uint32_t then) { return now-then; }
bool ring_push(struct ring *r, uint8_t value) {
    unsigned next = (r->head+1u)%8u;
    if (next == r->tail) return false;
    r->data[r->head] = value; r->head = next; return true;
}
bool ring_pop(struct ring *r, uint8_t *out) {
    if (!out || r->head == r->tail) return false;
    *out = r->data[r->tail]; r->tail = (r->tail+1u)%8u; return true;
}
bool debounce_update(struct debounce *d, bool raw, uint32_t now, uint32_t threshold) {
    if (raw != d->candidate) { d->candidate = raw; d->since = now; }
    if (raw != d->stable && elapsed_ticks(now,d->since) >= threshold) {
        d->stable = raw; return true;
    }
    return false;
}
void parser_feed(struct parser *p, const uint8_t *data, size_t n, frame_fn fn, void *ctx) {
    for (size_t i=0;i<n;++i) {
        uint8_t byte = data[i];
        if (p->state == 0) {
            if (byte > FRAME_MAX) { ++p->rejected; continue; }
            p->length=byte; p->used=0; p->checksum=byte;
            p->state=byte?1u:2u;
        } else if (p->state == 1) {
            p->body[p->used++]=byte; p->checksum^=byte;
            if (p->used == p->length) p->state=2;
        } else {
            if (byte == p->checksum) { if (fn) fn(ctx,p->body,p->length); }
            else ++p->rejected;
            p->state=0;
        }
    }
}
