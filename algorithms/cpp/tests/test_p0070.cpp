#include "../solutions/p0070_climbing_stairs.hpp"

static int slow(int n) { if(n<=1)return 1;return slow(n-1)+slow(n-2); }
int main() {
    hot100::p0070::Solution s;
    assert(s.climbStairs(1)==1);assert(s.climbStairs(2)==2);assert(s.climbStairs(6)==13);
    assert(s.climbStairs(0)==1);assert(s.climbStairs(45)==1836311903);
    for(int n=0;n<=23;++n)assert(s.climbStairs(n)==slow(n));
}
