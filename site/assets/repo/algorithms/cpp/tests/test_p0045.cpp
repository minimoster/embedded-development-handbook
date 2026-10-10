#include "../solutions/p0045_jump_game_ii.hpp"

static int slow(const std::vector<int>& a) {
    if(a.empty()) return 0;
    int n=static_cast<int>(a.size());std::vector<int> d(n,n+1);d[0]=0;
    for(int i=0;i<n;++i) for(int k=1;k<=a[i]&&i+k<n;++k) d[i+k]=std::min(d[i+k],d[i]+1);
    return d.back()==n+1 ? -1:d.back();
}
int main() {
    hot100::p0045::Solution s;
    std::vector<int> a{3,1,1,2,0,1}; assert(s.jump(a)==2);
    std::vector<int> one{0},no{1,0,2},empty;assert(s.jump(one)==0);assert(s.jump(no)==-1);assert(s.jump(empty)==0);
    for(int n=1;n<=8;++n) {
        int total=1;for(int i=0;i<n;++i)total*=4;
        for(int mask=0;mask<total;++mask){int k=mask;std::vector<int> v(n);for(int& x:v){x=k%4;k/=4;}assert(s.jump(v)==slow(v));}
    }
}
