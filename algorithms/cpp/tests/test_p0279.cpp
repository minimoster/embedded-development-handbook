#include "../solutions/p0279_perfect_squares.hpp"

static int bfs(int n) {
    std::vector<int> d(n+1,-1);std::queue<int> q;d[0]=0;q.push(0);
    while(!q.empty()){int x=q.front();q.pop();if(x==n)return d[x];
        for(int r=1;r*r+x<=n;++r){int y=x+r*r;if(d[y]<0){d[y]=d[x]+1;q.push(y);}}
    }return -1;
}
int main() {
    hot100::p0279::Solution s;
    assert(s.numSquares(13)==2);assert(s.numSquares(23)==4);assert(s.numSquares(0)==0);assert(s.numSquares(1)==1);
    for(int n=0;n<=400;++n)assert(s.numSquares(n)==bfs(n));
    assert(s.numSquares(10000)==1);
}
