#include "../examples/c/numeric_parse.h"
#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
static void valid(const char *s,int32_t expected){int32_t value=17;assert(parse_i32_span(s,strlen(s),&value));assert(value==expected);}
static void invalid(const char *s){int32_t value=17;assert(!parse_i32_span(s,strlen(s),&value));assert(value==17);}
int main(void){
    valid("2147483647",INT32_MAX);valid("-2147483648",INT32_MIN);valid("+0",0);valid("-0",0);valid("000123",123);
    invalid("2147483648");invalid("-2147483649");invalid("");invalid("+");invalid("-");invalid(" 1");invalid("1 ");invalid("1x");invalid("9999999999999999999999999");
    int32_t value=17;assert(!parse_i32_span(NULL,0,&value));assert(!parse_i32_span("1",1,NULL));
    const char embedded_zero[3]={'1',0,'2'};assert(!parse_i32_span(embedded_zero,3,&value));assert(value==17);
    assert(parse_i32_span("123trailing",3,&value)&&value==123);
    char buffer[32];
    for(int32_t i=-10000;i<=10000;++i){int len=snprintf(buffer,sizeof buffer,"%" PRId32,i);assert(len>0);assert(parse_i32_span(buffer,(size_t)len,&value));assert(value==i);}
    uint32_t rng=UINT32_C(1234567);
    /* Independent int64 oracle: generated magnitudes <= 12 decimal digits. */
    for(unsigned test=0;test<50000;++test){
        rng=rng*UINT32_C(1664525)+UINT32_C(1013904223);
        unsigned digits=1u+rng%12u;bool neg=(rng&16u)!=0;size_t used=0;int64_t magnitude=0;
        if(neg)buffer[used++]='-';
        for(unsigned j=0;j<digits;++j){rng=rng*UINT32_C(1664525)+UINT32_C(1013904223);unsigned digit=rng%10u;buffer[used++]=(char)('0'+digit);magnitude=magnitude*10+(int64_t)digit;}
        int64_t expected=neg?-magnitude:magnitude;value=17;
        bool ok=parse_i32_span(buffer,used,&value);bool in_range=expected>=INT32_MIN&&expected<=INT32_MAX;assert(ok==in_range);
        if(ok)assert(value==(int32_t)expected);else assert(value==17);
    }
    puts("Numeric parsing: PASS (20001 roundtrips, 50000 bounded independent oracle cases, extrema/invalid spans)");
}
