#include "../solutions/p0215_kth_largest_element_in_an_array.hpp"

int main() {
    hot100::p0215::Solution s;
    std::vector<int> a{9,2,9,4,1}; assert(s.findKthLargest(a,2)==9);
    std::vector<int> same(1000,7); assert(s.findKthLargest(same,900)==7);
    std::vector<int> extremes{INT_MIN,0,INT_MAX}; assert(s.findKthLargest(extremes,1)==INT_MAX);
    std::mt19937 rng(215);
    for(int z=0;z<800;++z) {
        std::vector<int> input(1+rng()%60); for(int& x:input) x=static_cast<int>(rng()%21)-10;
        auto sorted=input; std::sort(sorted.begin(),sorted.end());
        for(int k=1;k<=static_cast<int>(input.size());++k) { auto v=input; assert(s.findKthLargest(v,k)==sorted[sorted.size()-k]); }
    }
}
