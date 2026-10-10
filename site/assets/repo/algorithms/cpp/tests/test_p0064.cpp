#include "../solutions/p0064_minimum_path_sum.hpp"

static int slow(const std::vector<std::vector<int>>& a,int r=0,int c=0) {
    int m=static_cast<int>(a.size()),n=static_cast<int>(a[0].size());
    if(r==m-1&&c==n-1)return a[r][c];
    int best=INT_MAX;
    if(r+1<m)best=std::min(best,slow(a,r+1,c));
    if(c+1<n)best=std::min(best,slow(a,r,c+1));
    return a[r][c]+best;
}
int main() {
    hot100::p0064::Solution s;
    std::vector<std::vector<int>> a{{2,8,1},{1,3,2},{6,1,4}};auto old=a;assert(s.minPathSum(a)==11);assert(a==old);
    std::vector<std::vector<int>> one{{7}},row{{2,0,3}},empty;assert(s.minPathSum(one)==7);assert(s.minPathSum(row)==5);assert(s.minPathSum(empty)==0);
    std::mt19937 rng(64);
    for(int z=0;z<800;++z){int m=1+rng()%5,n=1+rng()%5;std::vector<std::vector<int>> v(m,std::vector<int>(n));for(auto& r:v)for(int& x:r)x=static_cast<int>(rng()%10);assert(s.minPathSum(v)==slow(v));}
}
