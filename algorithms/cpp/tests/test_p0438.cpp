#include "../solutions/p0438_find_all_anagrams_in_a_string.hpp"
using namespace hot100::p0438;
int main() {
Solution s;assert((s.findAnagrams("baaab","aab")==std::vector<int>{0,2}));assert((s.findAnagrams("aaaa","aa")==std::vector<int>{0,1,2}));assert(s.findAnagrams("a","aa").empty());assert(s.findAnagrams("abc","").empty());
std::mt19937 rng(438);for(int z=0;z<250;++z){std::string a(rng()%20,'a'),p(1+rng()%6,'a');for(auto&c:a)c=char('a'+rng()%4);for(auto&c:p)c=char('a'+rng()%4);auto key=p;std::sort(key.begin(),key.end());std::vector<int>b;for(int i=0;i+int(p.size())<=int(a.size());++i){auto v=a.substr(i,p.size());std::sort(v.begin(),v.end());if(v==key)b.push_back(i);}assert(s.findAnagrams(a,p)==b);}
}
