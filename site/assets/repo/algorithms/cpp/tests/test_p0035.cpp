#include "../solutions/p0035_search_insert_position.hpp"

int main() {
    hot100::p0035::Solution s;
    std::vector<int> a{2,5,8,13};
    assert(s.searchInsert(a,8)==2); assert(s.searchInsert(a,6)==2);
    assert(s.searchInsert(a,-1)==0); assert(s.searchInsert(a,20)==4);
    std::vector<int> empty; assert(s.searchInsert(empty,3)==0);
    for(int mask=0;mask<(1<<9);++mask) {
        std::vector<int> v; for(int i=0;i<9;++i) if(mask&(1<<i)) v.push_back(i*2-8);
        for(int t=-10;t<=10;++t) {
            int want=0; while(want<static_cast<int>(v.size()) && v[want]<t) ++want;
            assert(s.searchInsert(v,t)==want);
        }
    }
}
