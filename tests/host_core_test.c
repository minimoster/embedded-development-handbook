#include "../examples/c/embedded_core.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
struct output { size_t frames, bytes; uint8_t data[128]; };
static void collect(void *ctx,const uint8_t *p,size_t n) {
    struct output *o=ctx;
    assert(o->bytes+n <= sizeof o->data);
    memcpy(o->data+o->bytes,p,n); o->bytes+=n; ++o->frames;
}
static void test_bits_time(void) {
    uint32_t out=0;
    for (unsigned i=0;i<32;++i) { assert(make_mask(i,&out)); assert(out==(UINT32_C(1)<<i)); }
    assert(!make_mask(32,&out)); assert(!make_mask(0,NULL));
    assert(add_u32(UINT32_MAX,0,&out)&&out==UINT32_MAX);
    assert(!add_u32(UINT32_MAX,1,&out)); assert(!add_u32(1,1,NULL));
    const uint8_t bytes[]={0x12,0x34}; uint16_t word=0;
    assert(read_be16(bytes,2,&word)&&word==0x1234);
    assert(!read_be16(bytes,1,&word)); assert(!read_be16(NULL,2,&word));
    assert(elapsed_ticks(5,UINT32_MAX-4)==10);
}
static void test_ring(void) {
    struct ring r={0}; uint8_t x=0;
    assert(!ring_pop(&r,&x));
    for (unsigned i=0;i<7;++i) assert(ring_push(&r,(uint8_t)i));
    assert(!ring_push(&r,9));
    for (unsigned i=0;i<7;++i) { assert(ring_pop(&r,&x)); assert(x==i); }
    /* Deterministic model comparison exercises every wrap many times. */
    uint8_t model[7]; unsigned size=0; uint32_t rng=12345;
    for (unsigned i=0;i<50000;++i) {
        rng=rng*UINT32_C(1664525)+UINT32_C(1013904223);
        if (rng&0x10000) {
            bool ok=ring_push(&r,(uint8_t)rng); assert(ok==(size<7));
            if(ok)model[size++]=(uint8_t)rng;
        } else {
            bool ok=ring_pop(&r,&x); assert(ok==(size>0));
            if(ok){assert(x==model[0]); --size; memmove(model,model+1,size);}
        }
    }
}
static void test_parser(void) {
    const uint8_t wire[]={3,1,2,3,3,0,0,2,9,7,12};
    /* All 2^(11-1)=1024 partitions of this stream. */
    for(unsigned partition=0;partition<1024;++partition){
        struct parser p={0}; struct output o={0}; size_t start=0;
        for(size_t end=1;end<=sizeof wire;++end){
            if(end==sizeof wire || (partition&(1u<<(end-1)))) {
                parser_feed(&p,wire+start,end-start,collect,&o);start=end;
            }
        }
        assert(o.frames==3&&o.bytes==5&&p.rejected==0&&p.state==0);
        const uint8_t expected[]={1,2,3,9,7}; assert(memcmp(o.data,expected,5)==0);
    }
    struct parser p={0}; struct output o={0};
    const uint8_t bad[]={255,1,7,0};
    parser_feed(&p,bad,sizeof bad,collect,&o);
    assert(p.rejected==2&&o.frames==0);
    struct parser truncated={0}; parser_feed(&truncated,wire,2,collect,&o);
    assert(truncated.state==1&&o.frames==0); /* no invented complete frame */
    uint8_t maxframe[18]={16};
    for(unsigned i=0;i<16;++i)maxframe[1+i]=(uint8_t)i;
    maxframe[17]=16; /* XOR(0..15)==0 */
    struct parser m={0};struct output full={0};
    parser_feed(&m,maxframe,sizeof maxframe,collect,&full);assert(full.frames==1&&full.bytes==16);
}
static void test_debounce(void) {
    struct debounce d={false,false,0};
    assert(!debounce_update(&d,true,10,20));
    assert(!debounce_update(&d,false,15,20));
    assert(!debounce_update(&d,true,17,20));
    assert(!debounce_update(&d,true,36,20));
    assert(debounce_update(&d,true,37,20));
    assert(!debounce_update(&d,true,60,20));
    assert(!debounce_update(&d,false,UINT32_MAX-4,10));
    assert(debounce_update(&d,false,5,10));
}
int main(void){test_bits_time();test_ring();test_parser();test_debounce();puts("C core: PASS (bits/time, 50000 ring steps, 1024 frame partitions, debounce)");}
