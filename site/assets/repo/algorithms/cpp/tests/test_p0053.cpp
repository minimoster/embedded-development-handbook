#include "../solutions/p0053_maximum_subarray.hpp"
using namespace hot100::p0053;
int main() {
Solution s;std::vector<int>a{-4,5,-2,3,-6};assert(s.maxSubArray(a)==6);a={-8,-3,-5};assert(s.maxSubArray(a)==-3);a={};assert(s.maxSubArray(a)==0);
std::mt19937 rng(53);for(int z=0;z<300;++z){a.resize(1+rng()%20);for(auto&x:a)x=int(rng()%17)-8;int b=INT_MIN;for(int i=0;i<int(a.size());++i){int sum=0;for(int j=i;j<int(a.size());++j){sum+=a[j];b=std::max(b,sum);}}assert(s.maxSubArray(a)==b);}
}
