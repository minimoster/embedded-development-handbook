#include "../solutions/p0207_course_schedule.hpp"

bool reference(int n,const std::vector<std::vector<int>>& edges) {
    std::vector<std::vector<bool>> reach(n,std::vector<bool>(n));for(auto&e:edges)reach[e[1]][e[0]]=true;
    for(int k=0;k<n;++k)for(int i=0;i<n;++i)for(int j=0;j<n;++j)reach[i][j]=reach[i][j]||(reach[i][k]&&reach[k][j]);
    for(int i=0;i<n;++i) { if(reach[i][i]) return false; }
    return true;
}
int main() {
    hot100::p0207::Solution s;std::vector<std::vector<int>> cycle{{0,1},{1,0}};assert(!s.canFinish(2,cycle));
    std::vector<std::vector<int>> clean{{1,0}};assert(s.canFinish(2,clean));assert(s.canFinish(2,clean));
    std::vector<std::vector<int>> self{{0,0}};assert(!s.canFinish(1,self));std::vector<std::vector<int>> empty;assert(s.canFinish(6,empty));assert(s.canFinish(0,empty));
    std::vector<std::vector<int>> duplicate{{1,0},{1,0}};assert(s.canFinish(2,duplicate));
    std::mt19937 rng(207);for(int round=0;round<450;++round){int n=1+static_cast<int>(rng()%9);std::vector<std::vector<int>> edges;int e=static_cast<int>(rng()%25);for(int i=0;i<e;++i)edges.push_back({static_cast<int>(rng()%static_cast<unsigned>(n)),static_cast<int>(rng()%static_cast<unsigned>(n))});assert(s.canFinish(n,edges)==reference(n,edges));}
}
