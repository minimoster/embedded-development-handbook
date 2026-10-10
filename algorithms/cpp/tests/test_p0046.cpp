#include "../solutions/p0046_permutations.hpp"
using namespace hot100::p0046;
int main() {
Solution s;std::vector<int>a{2,5,8};auto r=s.permute(a);assert(r.size()==6);std::set<std::vector<int>>u(r.begin(),r.end());assert(u.size()==6);for(auto v:r){std::sort(v.begin(),v.end());assert(v==a);}a={4};assert((s.permute(a)==std::vector<std::vector<int>>{{4}}));a={};assert((s.permute(a)==std::vector<std::vector<int>>{{}}));
for(int n=1;n<=7;++n){a.resize(n);std::iota(a.begin(),a.end(),1);std::set<std::vector<int>>e;do{e.insert(a);}while(std::next_permutation(a.begin(),a.end()));r=s.permute(a);assert(std::set<std::vector<int>>(r.begin(),r.end())==e);assert(r.size()==e.size());}
}
