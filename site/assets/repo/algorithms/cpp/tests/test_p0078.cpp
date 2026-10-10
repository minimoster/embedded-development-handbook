#include "../solutions/p0078_subsets.hpp"
using namespace hot100::p0078;
int main() {
Solution s;std::vector<int>a{2,4,7};auto r=s.subsets(a);assert(r.size()==8);assert(std::set<std::vector<int>>(r.begin(),r.end()).size()==8);a={};assert((s.subsets(a)==std::vector<std::vector<int>>{{}}));
for(int n=0;n<=9;++n){a.resize(n);std::iota(a.begin(),a.end(),1);std::set<std::vector<int>>e;for(int mask=0;mask<(1<<n);++mask){std::vector<int>v;for(int i=0;i<n;++i)if(mask&(1<<i))v.push_back(a[i]);e.insert(v);}r=s.subsets(a);assert(std::set<std::vector<int>>(r.begin(),r.end())==e);assert(r.size()==e.size());}
}
