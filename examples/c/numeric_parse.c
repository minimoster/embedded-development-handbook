#include "numeric_parse.h"
#include <stdint.h>
#include <limits.h>
/* The public contract is ASCII octets; character literals require this target. */
_Static_assert(CHAR_BIT == 8 && '+' == 0x2b && '-' == 0x2d &&
               '0' == 0x30 && '9' == 0x39,
               "parse_i32_span requires 8-bit ASCII-compatible execution characters");
bool parse_i32_span(const char *data, size_t n, int32_t *out) {
    if (!data || !out || n == 0) return false;
    size_t i=0; bool negative=false;
    if (data[0]=='+' || data[0]=='-') { negative=data[0]=='-'; i=1; }
    if (i==n) return false;
    const uint32_t limit=negative ? (uint32_t)INT32_MAX+UINT32_C(1) : (uint32_t)INT32_MAX;
    uint32_t acc=0;
    for (;i<n;++i) {
        unsigned char c=(unsigned char)data[i];
        if (c<(unsigned char)'0' || c>(unsigned char)'9') return false;
        uint32_t digit=(uint32_t)(c-(unsigned char)'0');
        if (acc>(limit-digit)/UINT32_C(10)) return false;
        acc=acc*UINT32_C(10)+digit;
    }
    int32_t result;
    if (negative && acc==(uint32_t)INT32_MAX+UINT32_C(1)) result=INT32_MIN;
    else result=negative ? -(int32_t)acc : (int32_t)acc;
    *out=result; return true;
}
