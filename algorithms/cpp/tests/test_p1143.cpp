#include "../solutions/p1143_longest_common_subsequence.hpp"

static bool subsequence(const std::string& small,const std::string& big) {
    std::size_t p=0;for(char c:big)if(p<small.size()&&c==small[p])++p;return p==small.size();
}
static int slow(const std::string& a,const std::string& b) {
    int best=0;for(int mask=0;mask<(1<<a.size());++mask){std::string s;for(int i=0;i<static_cast<int>(a.size());++i)if(mask&(1<<i))s+=a[i];if(subsequence(s,b))best=std::max(best,static_cast<int>(s.size()));}return best;
}
int main() {
    hot100::p1143::Solution s;
    assert(s.longestCommonSubsequence("axbyc","abc")==3);assert(s.longestCommonSubsequence("aa","a")==1);
    assert(s.longestCommonSubsequence("","abc")==0);assert(s.longestCommonSubsequence("abc","xyz")==0);
    std::mt19937 rng(1143);for(int z=0;z<1500;++z){std::string a(rng()%10,'a'),b(rng()%10,'a');for(char& c:a)c=static_cast<char>('a'+rng()%3);for(char& c:b)c=static_cast<char>('a'+rng()%3);assert(s.longestCommonSubsequence(a,b)==slow(a,b));}
}
