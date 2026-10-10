#include "../solutions/p0118_pascals_triangle.hpp"

int main() {
    hot100::p0118::Solution s;
    assert((s.generate(4)==std::vector<std::vector<int>>{{1},{1,1},{1,2,1},{1,3,3,1}}));
    assert(s.generate(0).empty());
    for(int n=1;n<=30;++n) {
        auto rows=s.generate(n);assert(static_cast<int>(rows.size())==n);
        for(int r=0;r<n;++r) {
            assert(static_cast<int>(rows[r].size())==r+1);long long choose=1,total=0;
            for(int c=0;c<=r;++c) { assert(rows[r][c]==choose);total+=rows[r][c];choose=choose*(r-c)/(c+1); }
            assert(total==(1LL<<r));
        }
    }
}
