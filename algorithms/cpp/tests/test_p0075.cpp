#include "../solutions/p0075_sort_colors.hpp"

int main() {
    hot100::p0075::Solution s;
    std::vector<int> a{2,1,0,2,0,1};s.sortColors(a);assert((a==std::vector<int>{0,0,1,1,2,2}));
    std::vector<int> e;s.sortColors(e);assert(e.empty());
    for(int n=0;n<=10;++n){int total=1;for(int i=0;i<n;++i)total*=3;
        for(int mask=0;mask<total;++mask){int k=mask;std::vector<int> v(n);for(int& x:v){x=k%3;k/=3;}auto want=v;std::sort(want.begin(),want.end());s.sortColors(v);assert(v==want);}}
}
