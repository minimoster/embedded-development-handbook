#include "../solutions/p0153_find_minimum_in_rotated_sorted_array.hpp"

int main() {
    hot100::p0153::Solution s;
    std::vector<int> a{9,12,2,5,7}; assert(s.findMin(a)==2);
    std::vector<int> one{-8}; assert(s.findMin(one)==-8);
    for(int n=1;n<=40;++n) for(int shift=0;shift<n;++shift) {
        std::vector<int> v; for(int i=0;i<n;++i) v.push_back(i*2-40);
        std::rotate(v.begin(),v.begin()+shift,v.end());
        auto original=v; assert(s.findMin(v)==-40); assert(v==original);
    }
    bool caught=false; std::vector<int> empty;
    try { s.findMin(empty); } catch(const std::invalid_argument&) { caught=true; }
    assert(caught);
}
