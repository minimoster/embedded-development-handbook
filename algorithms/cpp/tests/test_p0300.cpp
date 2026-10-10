#include "../solutions/p0300_longest_increasing_subsequence.hpp"

static int slow(const std::vector<int>& a) {
    int n=static_cast<int>(a.size()),best=0;
    for(int mask=0;mask<(1<<n);++mask){bool valid=true,has=false;int prev=0,len=0;
        for(int i=0;i<n;++i) {
            if(mask&(1<<i)) { if(has&&a[i]<=prev)valid=false;prev=a[i];has=true;++len; }
        }
        if(valid)best=std::max(best,len);
    }
    return best;
}
int main() {
    hot100::p0300::Solution s;
    std::vector<int> a{4,10,5,6,2,8};assert(s.lengthOfLIS(a)==4);
    std::vector<int> equal{3,3,3},empty;assert(s.lengthOfLIS(equal)==1);assert(s.lengthOfLIS(empty)==0);
    std::mt19937 rng(300);
    for(int z=0;z<1100;++z){std::vector<int> v(rng()%12);for(int& x:v)x=static_cast<int>(rng()%15)-7;assert(s.lengthOfLIS(v)==slow(v));}
}
