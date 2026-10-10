#include "../solutions/p0031_next_permutation.hpp"

int main() {
    hot100::p0031::Solution s;
    std::vector<int> a{1,4,3,2};s.nextPermutation(a);assert((a==std::vector<int>{2,1,3,4}));
    std::vector<int> d{3,2,1};s.nextPermutation(d);assert((d==std::vector<int>{1,2,3}));
    std::vector<int> e,one{7};s.nextPermutation(e);s.nextPermutation(one);assert(e.empty());assert(one[0]==7);
    for(int n=0;n<=9;++n){int total=1;for(int i=0;i<n;++i)total*=3;
        for(int mask=0;mask<total;++mask){int k=mask;std::vector<int> v(n);for(int& x:v){x=k%3;k/=3;}auto want=v;std::next_permutation(want.begin(),want.end());s.nextPermutation(v);assert(v==want);}}
}
