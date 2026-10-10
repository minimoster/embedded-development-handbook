#include "../solutions/p0054_spiral_matrix.hpp"
using namespace hot100::p0054;
int main() {
Solution s;std::vector<std::vector<int>>a{{1,2,3},{4,5,6}};assert((s.spiralOrder(a)==std::vector<int>{1,2,3,6,5,4}));a={{1},{2},{3}};assert((s.spiralOrder(a)==std::vector<int>{1,2,3}));a={{1,2,3}};assert((s.spiralOrder(a)==std::vector<int>{1,2,3}));a={};assert(s.spiralOrder(a).empty());
for(int r=1;r<=6;++r)for(int c=1;c<=6;++c){a.assign(r,std::vector<int>(c));int v=0;for(auto&row:a)for(auto&x:row)x=++v;std::vector<int>b;std::vector<std::vector<bool>>seen(r,std::vector<bool>(c));int x=0,y=0,d=0;const int dx[]={0,1,0,-1},dy[]={1,0,-1,0};for(int k=0;k<r*c;++k){b.push_back(a[x][y]);seen[x][y]=true;int xx=x+dx[d],yy=y+dy[d];if(xx<0||xx>=r||yy<0||yy>=c||seen[xx][yy]){d=(d+1)%4;xx=x+dx[d];yy=y+dy[d];}x=xx;y=yy;}assert(s.spiralOrder(a)==b);}
}
