#include "../solutions/p0033_search_in_rotated_sorted_array.hpp"

int main() {
    hot100::p0033::Solution s;
    std::vector<int> v{8,11,15,1,4,6};
    assert(s.search(v,1)==3); assert(s.search(v,7)==-1);
    std::vector<int> empty; assert(s.search(empty,0)==-1);
    for(int n=1;n<=30;++n) for(int shift=0;shift<n;++shift) {
        std::vector<int> a; for(int i=0;i<n;++i) a.push_back(i*3-20);
        std::rotate(a.begin(),a.begin()+shift,a.end());
        for(int t=-23;t<=3*n;++t) {
            int want=-1; for(int i=0;i<n;++i) if(a[i]==t) want=i;
            assert(s.search(a,t)==want);
        }
    }
}
