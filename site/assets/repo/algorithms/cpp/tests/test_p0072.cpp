#include "../solutions/p0072_edit_distance.hpp"

static int recursive(const std::string& a,const std::string& b,int i,int j) {
    if(!i)return j;
    if(!j)return i;
    if(a[i-1]==b[j-1])return recursive(a,b,i-1,j-1);
    return 1+std::min({recursive(a,b,i-1,j),recursive(a,b,i,j-1),recursive(a,b,i-1,j-1)});
}
int main() {
    hot100::p0072::Solution s;
    assert(s.minDistance("cat","cut")==1);assert(s.minDistance("abc","yabd")==2);
    assert(s.minDistance("","abc")==3);assert(s.minDistance("same","same")==0);
    std::vector<std::string> all;
    for(int n=0;n<=4;++n)for(int mask=0;mask<(1<<n);++mask){std::string x(n,'a');for(int i=0;i<n;++i)if(mask&(1<<i))x[i]='b';all.push_back(x);}
    for(const auto& a:all)for(const auto& b:all){int got=s.minDistance(a,b);assert(got==recursive(a,b,static_cast<int>(a.size()),static_cast<int>(b.size())));assert(got==s.minDistance(b,a));}
}
