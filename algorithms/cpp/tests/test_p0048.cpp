#include "../solutions/p0048_rotate_image.hpp"
using namespace hot100::p0048;
int main() {
Solution s;std::vector<std::vector<int>>a{{1,2},{3,4}};s.rotate(a);assert((a==std::vector<std::vector<int>>{{3,1},{4,2}}));a={{7}};s.rotate(a);assert(a[0][0]==7);a={};s.rotate(a);
for(int n=1;n<=8;++n){a.assign(n,std::vector<int>(n));int v=0;for(auto&row:a)for(auto&x:row)x=++v;auto old=a,b=a;for(int i=0;i<n;++i)for(int j=0;j<n;++j)b[j][n-1-i]=a[i][j];s.rotate(a);assert(a==b);for(int k=0;k<3;++k)s.rotate(a);assert(a==old);}
}
