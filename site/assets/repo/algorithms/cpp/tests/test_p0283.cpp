#include "../solutions/p0283_move_zeroes.hpp"
using namespace hot100::p0283;
int main() {
Solution s;std::vector<int>a{0,4,0,-2,7,0};s.moveZeroes(a);assert((a==std::vector<int>{4,-2,7,0,0,0}));a={0,0};s.moveZeroes(a);assert((a==std::vector<int>{0,0}));a={};s.moveZeroes(a);assert(a.empty());
std::mt19937 rng(283);for(int z=0;z<300;++z){a.resize(rng()%30);for(auto&x:a)x=int(rng()%5)-2;std::vector<int>b;for(int x:a)if(x)b.push_back(x);b.resize(a.size(),0);s.moveZeroes(a);assert(a==b);}
}
