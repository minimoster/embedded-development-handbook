#ifndef NUMERIC_PARSE_H
#define NUMERIC_PARSE_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
/* Exact ASCII decimal span. Optional +/-; >=1 digit; no whitespace/trailing bytes.
 * Empty/NULL input rejected. On failure *out is unchanged. No allocation/locale.
 * A byte span need not be NUL-terminated; any NUL inside n is rejected.
 * Requires 8-bit bytes and ASCII-compatible execution characters; checked in .c. */
bool parse_i32_span(const char *data, size_t n, int32_t *out);
#endif
