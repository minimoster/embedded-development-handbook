#include "../solutions/p0200_number_of_islands.hpp"

int reference(const std::vector<std::vector<char>>& g) {
    if(g.empty()||g[0].empty()) return 0;
    int r=static_cast<int>(g.size()),c=static_cast<int>(g[0].size());std::vector<int> parent(r*c);std::iota(parent.begin(),parent.end(),0);
    std::function<int(int)> find=[&](int x){while(parent[x]!=x)x=parent[x];return x;};
    for(int i=0;i<r;++i)for(int j=0;j<c;++j)if(g[i][j]=='1'){
        if(i+1<r&&g[i+1][j]=='1')parent[find(i*c+j)]=find((i+1)*c+j);
        if(j+1<c&&g[i][j+1]=='1')parent[find(i*c+j)]=find(i*c+j+1);
    }
    std::set<int> roots;for(int i=0;i<r;++i)for(int j=0;j<c;++j)if(g[i][j]=='1')roots.insert(find(i*c+j));return static_cast<int>(roots.size());
}
int main() {
    hot100::p0200::Solution s;std::vector<std::vector<char>> empty;assert(s.numIslands(empty)==0);
    std::vector<std::vector<char>> diagonal{{'1','0'},{'0','1'}};assert(s.numIslands(diagonal)==2);assert(s.numIslands(diagonal)==0);
    std::mt19937 rng(200);for(int round=0;round<350;++round){int r=1+static_cast<int>(rng()%10),c=1+static_cast<int>(rng()%12);std::vector<std::vector<char>> grid(r,std::vector<char>(c));for(auto&row:grid)for(auto&x:row)x=rng()%2?'1':'0';int expected=reference(grid);assert(s.numIslands(grid)==expected);for(auto&row:grid)for(char x:row)assert(x=='0');}
    std::vector<std::vector<char>> strip(1,std::vector<char>(10000,'1'));assert(s.numIslands(strip)==1);
}
