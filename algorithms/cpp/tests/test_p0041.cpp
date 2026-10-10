#include "../solutions/p0041_first_missing_positive.hpp"
using namespace hot100::p0041;
int main() {
Solution s;std::vector<int>a{4,-1,1,2};assert(s.firstMissingPositive(a)==3);a={1,1};assert(s.firstMissingPositive(a)==2);a={};assert(s.firstMissingPositive(a)==1);a={INT_MIN,INT_MAX};assert(s.firstMissingPositive(a)==1);
std::mt19937 rng(41);for(int z=0;z<300;++z){a.resize(rng()%22);for(auto&x:a)x=int(rng()%31)-10;std::set<int>values(a.begin(),a.end());int b=1;while(values.count(b))++b;assert(s.firstMissingPositive(a)==b);}
}
