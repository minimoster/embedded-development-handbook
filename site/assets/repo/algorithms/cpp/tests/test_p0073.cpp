#include "../solutions/p0073_set_matrix_zeroes.hpp"
using namespace hot100::p0073;
int main() {
Solution s;std::vector<std::vector<int>>a{{1,2,3},{4,0,6},{7,8,9}};s.setZeroes(a);assert((a==std::vector<std::vector<int>>{{1,0,3},{0,0,0},{7,0,9}}));a={{0,1,2}};s.setZeroes(a);assert((a==std::vector<std::vector<int>>{{0,0,0}}));a={};s.setZeroes(a);
std::mt19937 rng(73);for(int z=0;z<250;++z){int r=1+rng()%6,c=1+rng()%6;a.assign(r,std::vector<int>(c));for(auto&row:a)for(auto&x:row)x=rng()%4;auto b=a;std::vector<bool>rr(r),cc(c);for(int i=0;i<r;++i)for(int j=0;j<c;++j)if(a[i][j]==0)rr[i]=cc[j]=true;for(int i=0;i<r;++i)for(int j=0;j<c;++j)if(rr[i]||cc[j])b[i][j]=0;s.setZeroes(a);assert(a==b);}
}
