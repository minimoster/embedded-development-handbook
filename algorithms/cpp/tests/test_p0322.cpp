#include "../solutions/p0322_coin_change.hpp"

static int bfs(const std::vector<int>& coins,int amount) {
    std::vector<int> d(amount+1,-1);std::queue<int> q;d[0]=0;q.push(0);
    while(!q.empty()){int x=q.front();q.pop();if(x==amount)return d[x];for(int c:coins)if(c>0&&c<=amount-x&&d[x+c]<0){d[x+c]=d[x]+1;q.push(x+c);}}
    return -1;
}
int main() {
    hot100::p0322::Solution s;
    std::vector<int> a{1,4,6};assert(s.coinChange(a,8)==2);
    std::vector<int> b{4,6};assert(s.coinChange(b,7)==-1);assert(s.coinChange(b,0)==0);
    std::vector<int> empty;assert(s.coinChange(empty,3)==-1);
    std::mt19937 rng(322);
    for(int z=0;z<1200;++z){std::vector<int> v(1+rng()%6);for(int& c:v)c=1+static_cast<int>(rng()%12);int a=static_cast<int>(rng()%70);assert(s.coinChange(v,a)==bfs(v,a));}
}
