#include "../solutions/p0001_two_sum.hpp"
using namespace hot100::p0001;
int main() {
Solution s; std::vector<int>a{4,9,6}; assert((s.twoSum(a,10)==std::vector<int>{0,2}));
a={5,5};assert((s.twoSum(a,10)==std::vector<int>{0,1}));a={};assert(s.twoSum(a,0).empty());
a={INT_MIN,INT_MAX,-1};assert((s.twoSum(a,-1)==std::vector<int>{0,1}));
std::mt19937 rng(101);for(int z=0;z<250;++z){int n=2+rng()%15;a.resize(n);for(auto&x:a)x=int(rng()%21)-10;int i=rng()%n,j=(i+1+rng()%(n-1))%n;int t=a[i]+a[j];auto r=s.twoSum(a,t);assert(r.size()==2&&r[0]!=r[1]&&a[r[0]]+a[r[1]]==t);}
}
