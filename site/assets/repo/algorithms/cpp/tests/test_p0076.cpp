#include "../solutions/p0076_minimum_window_substring.hpp"
using namespace hot100::p0076;
int main() {
Solution s;assert(s.minWindow("cabbaa","aba")=="baa");assert(s.minWindow("x","xx")=="");assert(s.minWindow("","")=="");assert(s.minWindow("zab","ab")=="ab");
std::mt19937 rng(76);for(int z=0;z<250;++z){std::string a(rng()%14,'a'),t(1+rng()%4,'a');for(auto&c:a)c=char('a'+rng()%3);for(auto&c:t)c=char('a'+rng()%3);int best=INT_MAX;std::string exp;std::array<int,256>req{};for(unsigned char c:t)++req[c];for(int i=0;i<int(a.size());++i){std::array<int,256>w{};for(int j=i;j<int(a.size());++j){++w[static_cast<unsigned char>(a[j])];bool ok=true;for(int c=0;c<256;++c)if(w[c]<req[c])ok=false;if(ok){if(j-i+1<best){best=j-i+1;exp=a.substr(i,best);}break;}}}assert(s.minWindow(a,t)==exp);}
}
