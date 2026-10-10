#include "../solutions/p0240_search_a_2d_matrix_ii.hpp"
using namespace hot100::p0240;
int main() {
Solution s;std::vector<std::vector<int>>a{{1,4,8},{2,5,9},{3,7,12}};assert(s.searchMatrix(a,7));assert(!s.searchMatrix(a,6));a={};assert(!s.searchMatrix(a,1));a={{2,2}};assert(s.searchMatrix(a,2));
for(int r=1;r<=7;++r)for(int c=1;c<=7;++c){a.assign(r,std::vector<int>(c));for(int i=0;i<r;++i)for(int j=0;j<c;++j)a[i][j]=i*3+j*2;for(int t=-1;t<=r*3+c*2+1;++t){bool b=false;for(auto&row:a)for(int x:row)b|=x==t;assert(s.searchMatrix(a,t)==b);}}
}
