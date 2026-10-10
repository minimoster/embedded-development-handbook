#include "../solutions/p0042_trapping_rain_water.hpp"
using namespace hot100::p0042;
int main() {
Solution s;std::vector<int>a{3,0,2,0,4};assert(s.trap(a)==7);a={3,2,1};assert(s.trap(a)==0);a={};assert(s.trap(a)==0);
std::mt19937 rng(42);for(int z=0;z<300;++z){a.resize(rng()%25);for(auto&x:a)x=rng()%10;int b=0;for(int i=0;i<int(a.size());++i){int l=0,r=0;for(int j=0;j<=i;++j)l=std::max(l,a[j]);for(int j=i;j<int(a.size());++j)r=std::max(r,a[j]);b+=std::min(l,r)-a[i];}assert(s.trap(a)==b);}
}
