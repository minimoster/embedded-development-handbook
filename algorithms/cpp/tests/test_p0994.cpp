#include "../solutions/p0994_rotting_oranges.hpp"

int reference(std::vector<std::vector<int>> g) {
    if(g.empty()||g[0].empty()) return 0;
    int rows=static_cast<int>(g.size()),cols=static_cast<int>(g[0].size());int minutes=0;
    while(true){std::vector<std::pair<int,int>> change;bool fresh=false;
        for(int r=0;r<rows;++r)for(int c=0;c<cols;++c)if(g[r][c]==1){fresh=true;if((r>0&&g[r-1][c]==2)||(r+1<rows&&g[r+1][c]==2)||(c>0&&g[r][c-1]==2)||(c+1<cols&&g[r][c+1]==2))change.emplace_back(r,c);}
        if(!fresh) return minutes;
        if(change.empty()) return -1;
        for(auto [r,c]:change) { g[r][c]=2; }
        ++minutes;
    }
}
int main() {
    hot100::p0994::Solution s;std::vector<std::vector<int>> empty;assert(s.orangesRotting(empty)==0);
    std::vector<std::vector<int>> noFresh{{0,2,0}};assert(s.orangesRotting(noFresh)==0);
    std::vector<std::vector<int>> impossible{{2,0,1}};assert(s.orangesRotting(impossible)==-1);
    std::vector<std::vector<int>> line(1,std::vector<int>(25,1));line[0][0]=2;assert(s.orangesRotting(line)==24);
    std::vector<std::vector<int>> multi{{2,1,1,1,2},{0,1,1,1,0}};assert(s.orangesRotting(multi)==3);
    std::mt19937 rng(994);for(int round=0;round<400;++round){int r=1+static_cast<int>(rng()%8),c=1+static_cast<int>(rng()%13);std::vector<std::vector<int>> g(r,std::vector<int>(c));for(auto&row:g)for(auto&x:row)x=static_cast<int>(rng()%3);int expected=reference(g);assert(s.orangesRotting(g)==expected);}
}
