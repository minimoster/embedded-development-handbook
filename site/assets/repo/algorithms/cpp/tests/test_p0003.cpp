#include "../solutions/p0003_longest_substring_without_repeating_characters.hpp"
using namespace hot100::p0003;
int main() {
Solution s;assert(s.lengthOfLongestSubstring("tmmzuxt")==5);assert(s.lengthOfLongestSubstring("abba")==2);assert(s.lengthOfLongestSubstring("")==0);std::string bytes;bytes.push_back(char(255));bytes.push_back('a');bytes.push_back(char(255));assert(s.lengthOfLongestSubstring(bytes)==2);
std::mt19937 rng(3);for(int z=0;z<250;++z){std::string a(rng()%18,'a');for(auto&c:a)c=char('a'+rng()%5);int b=0;for(int i=0;i<int(a.size());++i){std::set<char>v;for(int j=i;j<int(a.size())&&v.insert(a[j]).second;++j)b=std::max(b,j-i+1);}assert(s.lengthOfLongestSubstring(a)==b);}
}
