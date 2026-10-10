#include "../solutions/p0152_maximum_product_subarray.hpp"

static int slow(const std::vector<int>& a) {
    long long best=std::numeric_limits<long long>::lowest();
    for(int i=0;i<static_cast<int>(a.size());++i){long long p=1;for(int j=i;j<static_cast<int>(a.size());++j){p*=a[j];best=std::max(best,p);}}
    return static_cast<int>(best);
}
int main() {
    hot100::p0152::Solution s;
    std::vector<int> a{-2,3,-4};assert(s.maxProduct(a)==24);
    std::vector<int> b{-3,0,-2},one{-5};assert(s.maxProduct(b)==0);assert(s.maxProduct(one)==-5);
    std::mt19937 rng(152);
    for(int z=0;z<2500;++z){std::vector<int> v(1+rng()%12);for(int& x:v)x=static_cast<int>(rng()%5)-2;assert(s.maxProduct(v)==slow(v));}
}
