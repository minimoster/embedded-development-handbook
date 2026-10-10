#include "../solutions/p0051_n_queens.hpp"
using namespace hot100::p0051;
int main() {
Solution s;const int counts[]={0,1,0,0,2,10,4,40,92};for(int n=1;n<=8;++n){auto r=s.solveNQueens(n);assert(int(r.size())==counts[n]);assert(std::set<std::vector<std::string>>(r.begin(),r.end()).size()==r.size());for(auto&b:r){std::vector<int>c;for(int i=0;i<n;++i){assert(std::count(b[i].begin(),b[i].end(),'Q')==1);c.push_back(int(b[i].find('Q')));}for(int i=0;i<n;++i)for(int j=i+1;j<n;++j)assert(c[i]!=c[j]&&std::abs(c[i]-c[j])!=j-i);}}assert(s.solveNQueens(0).empty());
}
