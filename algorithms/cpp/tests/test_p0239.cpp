#include "../solutions/p0239_sliding_window_maximum.hpp"
using namespace hot100::p0239;
int main() {
Solution s;std::vector<int>a{4,2,2,7,1};assert((s.maxSlidingWindow(a,3)==std::vector<int>{4,7,7}));assert(s.maxSlidingWindow(a,1)==a);assert((s.maxSlidingWindow(a,5)==std::vector<int>{7}));assert(s.maxSlidingWindow(a,0).empty());
std::mt19937 rng(239);for(int z=0;z<300;++z){a.resize(1+rng()%25);for(auto&x:a)x=int(rng()%19)-9;int k=1+rng()%a.size();std::vector<int>b;for(int i=0;i+k<=int(a.size());++i)b.push_back(*std::max_element(a.begin()+i,a.begin()+i+k));assert(s.maxSlidingWindow(a,k)==b);}
}
