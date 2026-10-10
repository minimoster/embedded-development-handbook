#include "../solutions/p0189_rotate_array.hpp"
using namespace hot100::p0189;
int main() {
Solution s;std::vector<int>a{1,2,3,4,5};s.rotate(a,7);assert((a==std::vector<int>{4,5,1,2,3}));s.rotate(a,0);assert((a==std::vector<int>{4,5,1,2,3}));a={};s.rotate(a,3);assert(a.empty());
std::mt19937 rng(189);for(int z=0;z<250;++z){a.resize(1+rng()%20);std::iota(a.begin(),a.end(),0);int k=rng()%70;auto b=a;for(int i=0;i<int(a.size());++i)b[(i+k)%a.size()]=a[i];s.rotate(a,k);assert(a==b);}
}
