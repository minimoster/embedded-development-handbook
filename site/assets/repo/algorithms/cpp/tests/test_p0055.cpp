#include "../solutions/p0055_jump_game.hpp"

static bool slow(const std::vector<int>& a) {
    if(a.empty()) return false;
    std::vector<unsigned char> reach(a.size()); reach[0]=1;
    for(int i=0;i<static_cast<int>(a.size());++i) if(reach[i])
        for(int d=1;d<=a[i] && i+d<static_cast<int>(a.size());++d) reach[i+d]=1;
    return reach.back()!=0;
}
int main() {
    hot100::p0055::Solution s;
    std::vector<int> yes{2,0,3,0,0,0},no{1,0,2},one{0},empty;
    assert(s.canJump(yes)); assert(!s.canJump(no)); assert(s.canJump(one)); assert(!s.canJump(empty));
    for(int n=1;n<=8;++n) {
        int total=1;for(int i=0;i<n;++i) total*=4;
        for(int mask=0;mask<total;++mask) { int k=mask;std::vector<int> a(n);for(int& x:a){x=k%4;k/=4;}assert(s.canJump(a)==slow(a)); }
    }
}
