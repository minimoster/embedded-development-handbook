#include "../solutions/p0034_find_first_and_last_position_of_element_in_sorted_array.hpp"

int main() {
    hot100::p0034::Solution s;
    std::vector<int> a{1,3,3,3,8};
    assert((s.searchRange(a,3)==std::vector<int>{1,3}));
    assert((s.searchRange(a,2)==std::vector<int>{-1,-1}));
    std::vector<int> empty; assert((s.searchRange(empty,0)==std::vector<int>{-1,-1}));
    std::vector<int> extremes{INT_MIN,INT_MIN,INT_MAX};
    assert((s.searchRange(extremes,INT_MAX)==std::vector<int>{2,2}));
    std::mt19937 rng(34);
    for(int z=0;z<1000;++z) {
        std::vector<int> v(rng()%40); for(int& x:v) x=static_cast<int>(rng()%11)-5;
        std::sort(v.begin(),v.end());
        for(int t=-6;t<=6;++t) {
            std::vector<int> want{-1,-1};
            for(int i=0;i<static_cast<int>(v.size());++i) if(v[i]==t) { if(want[0]<0) want[0]=i; want[1]=i; }
            assert(s.searchRange(v,t)==want);
        }
    }
}
