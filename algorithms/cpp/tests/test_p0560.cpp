#include "../solutions/p0560_subarray_sum_equals_k.hpp"
using namespace hot100::p0560;
int main() {
Solution s;std::vector<int>a{2,-2,2};assert(s.subarraySum(a,2)==3);a={0,0,0};assert(s.subarraySum(a,0)==6);a={};assert(s.subarraySum(a,0)==0);
std::mt19937 rng(560);for(int z=0;z<300;++z){a.resize(rng()%20);for(auto&x:a)x=int(rng()%9)-4;int k=int(rng()%11)-5,b=0;for(int i=0;i<int(a.size());++i){int sum=0;for(int j=i;j<int(a.size());++j){sum+=a[j];b+=sum==k;}}assert(s.subarraySum(a,k)==b);}
}
