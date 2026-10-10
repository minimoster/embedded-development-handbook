#include "../solutions/p0128_longest_consecutive_sequence.hpp"
using namespace hot100::p0128;
int main() {
Solution s;std::vector<int>a{8,3,4,2,3,9};assert(s.longestConsecutive(a)==3);a={};assert(s.longestConsecutive(a)==0);a={INT_MIN,INT_MIN+1,INT_MAX};assert(s.longestConsecutive(a)==2);
std::mt19937 rng(128);for(int z=0;z<300;++z){a.resize(rng()%25);for(auto&x:a)x=int(rng()%31)-15;auto b=a;std::sort(b.begin(),b.end());b.erase(std::unique(b.begin(),b.end()),b.end());int best=0,len=0;for(int i=0;i<int(b.size());++i){len=(i&&b[i]==b[i-1]+1)?len+1:1;best=std::max(best,len);}assert(s.longestConsecutive(a)==best);}
}
