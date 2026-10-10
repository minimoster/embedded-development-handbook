#include "../solutions/p0074_search_a_2d_matrix.hpp"

int main() {
    hot100::p0074::Solution s;
    std::vector<std::vector<int>> a{{-5,-2,1},{4,8,12}};
    assert(s.searchMatrix(a,4)); assert(!s.searchMatrix(a,3));
    assert(!s.searchMatrix(a,-8)); assert(!s.searchMatrix(a,18));
    std::vector<std::vector<int>> empty, row{{}};
    assert(!s.searchMatrix(empty,0)); assert(!s.searchMatrix(row,0));
    for(int m=1;m<=7;++m) for(int n=1;n<=7;++n) {
        std::vector<std::vector<int>> v(m,std::vector<int>(n));
        for(int i=0;i<m;++i) for(int j=0;j<n;++j) v[i][j]=3*(i*n+j)-20;
        for(int t=-22;t<=3*m*n;++t) {
            bool want=false; for(auto& r:v) for(int x:r) want=want||x==t;
            assert(s.searchMatrix(v,t)==want);
        }
    }
}
