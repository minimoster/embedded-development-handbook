#include "../solutions/p0005_longest_palindromic_substring.hpp"

static bool isPalindrome(const std::string& s) {return std::equal(s.begin(),s.end(),s.rbegin());}
static std::string slow(const std::string& s) {
    std::string best;
    for(int i=0;i<static_cast<int>(s.size());++i)for(int len=1;i+len<=static_cast<int>(s.size());++len){auto sub=s.substr(i,len);if(isPalindrome(sub)&&sub.size()>best.size())best=sub;}
    return best;
}
int main() {
    hot100::p0005::Solution sol;
    assert(sol.longestPalindrome("xabbaq")=="abba");assert(sol.longestPalindrome("abaqcdc")=="aba");
    assert(sol.longestPalindrome("")=="");assert(sol.longestPalindrome("a")=="a");assert(sol.longestPalindrome("aaaa")=="aaaa");
    for(int n=0;n<=10;++n)for(int mask=0;mask<(1<<n);++mask){std::string s(n,'a');for(int i=0;i<n;++i)if(mask&(1<<i))s[i]='b';assert(sol.longestPalindrome(s)==slow(s));}
}
