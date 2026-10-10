#include "../solutions/p0062_unique_paths.hpp"

static int slow(int m,int n) {
    if(m<=0||n<=0)return 0;
    if(m==1||n==1)return 1;
    return slow(m-1,n)+slow(m,n-1);
}
int main() {
    hot100::p0062::Solution s;
    assert(s.uniquePaths(3,4)==10);assert(s.uniquePaths(1,20)==1);assert(s.uniquePaths(0,4)==0);
    for(int m=1;m<=8;++m)for(int n=1;n<=8;++n){assert(s.uniquePaths(m,n)==slow(m,n));assert(s.uniquePaths(m,n)==s.uniquePaths(n,m));}
    assert(s.uniquePaths(17,17)==601080390);
}
