#include "../solutions/p0039_combination_sum.hpp"
using namespace hot100::p0039;
int main() {
Solution s;std::vector<int>a{3,5,7};auto r=s.combinationSum(a,10);std::sort(r.begin(),r.end());assert((r==std::vector<std::vector<int>>{{3,7},{5,5}}));a={4,6};assert(s.combinationSum(a,5).empty());a={2};assert((s.combinationSum(a,6)==std::vector<std::vector<int>>{{2,2,2}}));assert((s.combinationSum(a,0)==std::vector<std::vector<int>>{{}}));
for(int t=1;t<=18;++t){a={2,3,5};std::set<std::vector<int>>e;for(int x=0;x*2<=t;++x)for(int y=0;x*2+y*3<=t;++y){int rem=t-x*2-y*3;if(rem%5==0){std::vector<int>v(x,2);v.insert(v.end(),y,3);v.insert(v.end(),rem/5,5);e.insert(v);}}r=s.combinationSum(a,t);assert(std::set<std::vector<int>>(r.begin(),r.end())==e);assert(r.size()==e.size());}
}
