#include "../solutions/p0011_container_with_most_water.hpp"
using namespace hot100::p0011;
int main() {
Solution s;std::vector<int>a{2,6,3,5,4};assert(s.maxArea(a)==12);a={0,0};assert(s.maxArea(a)==0);a={};assert(s.maxArea(a)==0);
std::mt19937 rng(11);for(int z=0;z<300;++z){a.resize(2+rng()%15);for(auto&x:a)x=rng()%20;int b=0;for(int i=0;i<int(a.size());++i)for(int j=i+1;j<int(a.size());++j)b=std::max(b,(j-i)*std::min(a[i],a[j]));assert(s.maxArea(a)==b);}
}
