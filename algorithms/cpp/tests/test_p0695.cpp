#include "../supplementary/p0695_max_area_of_island.hpp"
using namespace hot100::p0695;
int main() {
Solution s;std::vector<std::vector<int>>a{{1,1,0},{0,1,0},{1,0,1}};assert(s.maxAreaOfIsland(a)==3);assert(s.maxAreaOfIsland(a)==0);a={{0,0}};assert(s.maxAreaOfIsland(a)==0);a={};assert(s.maxAreaOfIsland(a)==0);
std::mt19937 rng(695);for(int z=0;z<250;++z){int rows=1+rng()%6,cols=1+rng()%6;a.assign(rows,std::vector<int>(cols));for(auto&row:a)for(auto&x:row)x=rng()%2;std::vector<int>parent(rows*cols),size(rows*cols,1);std::iota(parent.begin(),parent.end(),0);std::function<int(int)>find=[&](int x){return parent[x]==x?x:parent[x]=find(parent[x]);};for(int r=0;r<rows;++r)for(int c=0;c<cols;++c)if(a[r][c]){for(auto [rr,cc]:std::vector<std::pair<int,int>>{{r+1,c},{r,c+1}})if(rr<rows&&cc<cols&&a[rr][cc]){int u=find(r*cols+c),v=find(rr*cols+cc);if(u!=v){parent[u]=v;size[v]+=size[u];}}}int expected=0;for(int r=0;r<rows;++r)for(int c=0;c<cols;++c)if(a[r][c])expected=std::max(expected,size[find(r*cols+c)]);assert(s.maxAreaOfIsland(a)==expected);}
}
