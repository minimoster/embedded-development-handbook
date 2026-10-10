#include "../solutions/p0015_3sum.hpp"
using namespace hot100::p0015;
int main() {
Solution s;std::vector<int>a{-3,-1,0,1,2,3};auto r=s.threeSum(a);std::sort(r.begin(),r.end());assert((r==std::vector<std::vector<int>>{{-3,0,3},{-3,1,2},{-1,0,1}}));a={0,0,0,0};assert((s.threeSum(a)==std::vector<std::vector<int>>{{0,0,0}}));a={};assert(s.threeSum(a).empty());
std::mt19937 rng(15);for(int z=0;z<250;++z){a.resize(rng()%12);for(auto&x:a)x=int(rng()%11)-5;std::set<std::vector<int>>b;for(int i=0;i<int(a.size());++i)for(int j=i+1;j<int(a.size());++j)for(int k=j+1;k<int(a.size());++k)if(a[i]+a[j]+a[k]==0){std::vector<int>v{a[i],a[j],a[k]};std::sort(v.begin(),v.end());b.insert(v);}auto got=s.threeSum(a);assert(std::set<std::vector<int>>(got.begin(),got.end())==b);assert(got.size()==b.size());}
}
