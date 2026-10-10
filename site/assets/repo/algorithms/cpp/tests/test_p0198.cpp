#include "../solutions/p0198_house_robber.hpp"

static int slow(const std::vector<int>& a) {
    int n=static_cast<int>(a.size()),best=0;
    for(int mask=0;mask<(1<<n);++mask) { if(mask&(mask<<1))continue;int sum=0;for(int i=0;i<n;++i)if(mask&(1<<i))sum+=a[i];best=std::max(best,sum); }
    return best;
}
int main() {
    hot100::p0198::Solution s;
    std::vector<int> a{4,1,2,9,3};assert(s.rob(a)==13);
    std::vector<int> e,one{7},zero{0,0,0},prefix{4,1};assert(s.rob(e)==0);assert(s.rob(one)==7);assert(s.rob(zero)==0);assert(s.rob(prefix)==4);
    std::mt19937 rng(198);
    for(int z=0;z<700;++z) {std::vector<int> v(rng()%13);for(int& x:v)x=static_cast<int>(rng()%31);assert(s.rob(v)==slow(v));}
}
