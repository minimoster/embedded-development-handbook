#include "../solutions/p0238_product_of_array_except_self.hpp"
using namespace hot100::p0238;
int main() {
Solution s;std::vector<int>a{2,3,4};assert((s.productExceptSelf(a)==std::vector<int>{12,8,6}));a={0,2,3};assert((s.productExceptSelf(a)==std::vector<int>{6,0,0}));a={0,2,0};assert((s.productExceptSelf(a)==std::vector<int>{0,0,0}));a={};assert(s.productExceptSelf(a).empty());
std::mt19937 rng(238);for(int z=0;z<250;++z){a.resize(2+rng()%8);for(auto&x:a)x=int(rng()%5)-2;std::vector<int>b(a.size(),1);for(int i=0;i<int(a.size());++i)for(int j=0;j<int(a.size());++j)if(i!=j)b[i]*=a[j];assert(s.productExceptSelf(a)==b);}
}
