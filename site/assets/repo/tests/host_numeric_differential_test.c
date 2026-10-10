#include "../examples/c/numeric_parse.h"
#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Finite host differential fixture; syntax is exact ASCII, range oracle is strtoimax. */
static uint64_t rng = UINT64_C(0x1862375ac917bd42);
static uint64_t next(void) {
    rng ^= rng << 13;
    rng ^= rng >> 7;
    rng ^= rng << 17;
    return rng;
}
static size_t checks;
static void check(const char *data, size_t n) {
    char buf[512];
    assert(n < sizeof buf);
    memcpy(buf, data, n);
    buf[n] = 0;
    size_t i = 0;
    if (n && (buf[0] == '+' || buf[0] == '-')) i = 1;
    bool syntax = i < n;
    for (; i < n; ++i) {
        if ((unsigned char)buf[i] < 0x30u || (unsigned char)buf[i] > 0x39u) syntax = false;
    }
    errno = 0;
    char *end = NULL;
    intmax_t oracle = syntax ? strtoimax(buf, &end, 10) : 0;
    bool expected = syntax && errno == 0 && end == buf + n && oracle >= INT32_MIN && oracle <= INT32_MAX;
    int32_t output = 12345;
    bool actual = parse_i32_span(data, n, &output);
    assert(actual == expected);
    assert(output == (expected ? (int32_t)oracle : 12345));
    ++checks;
}
int main(void) {
    char buf[512];
    for (int64_t base = (int64_t)INT32_MIN; base <= (int64_t)INT32_MAX; base += (int64_t)UINT32_MAX) {
        for (int64_t offset = -4096; offset <= 4096; ++offset) {
            int n = snprintf(buf, sizeof buf, "%" PRId64, base + offset);
            assert(n > 0 && (size_t)n < sizeof buf);
            check(buf, (size_t)n);
        }
    }
    const char *spans[] = {"+2147483647", "+2147483648", "-2147483648", "-2147483649", "++1", "--1", "+-1", "-+1", "1\n", "\t1", "", "+", "-"};
    for (size_t i = 0; i < sizeof spans / sizeof spans[0]; ++i) check(spans[i], strlen(spans[i]));
    memset(buf, '0', 510); check(buf, 510); buf[0] = '-'; check(buf, 510); buf[509] = '1'; check(buf, 510);
    for (unsigned test = 0; test < 100000; ++test) {
        size_t n = (size_t)(next() % 128u);
        for (size_t j = 0; j < n; ++j) buf[j] = (char)(unsigned char)next();
        check(buf, n);
    }
    for (unsigned test = 0; test < 100000; ++test) {
        size_t n = 1u + (size_t)(next() % 100u);
        for (size_t j = 0; j < n; ++j) buf[j] = (char)('0' + next() % 10u);
        if (next() & 1u) buf[0] = (next() & 1u) ? '+' : '-';
        check(buf, n);
    }
    const char exact[3] = {'1', '2', '3'}; check(exact, sizeof exact);
    int32_t untouched = 12345;
    assert(!parse_i32_span(NULL, 1, &untouched) && untouched == 12345);
    assert(!parse_i32_span("123", 3, NULL));
    printf("Numeric differential: PASS (%zu strtoimax-oracle span comparisons; null contracts)\n", checks);
    return 0;
}
