#include "../solutions/p0056_merge_intervals.hpp"
using namespace hot100::p0056;
int main() {
Solution s;std::vector<std::vector<int>>a{{5,8},{1,3},{3,6},{10,11}};assert((s.merge(a)==std::vector<std::vector<int>>{{1,8},{10,11}}));a={{1,9},{2,3},{1,1}};assert((s.merge(a)==std::vector<std::vector<int>>{{1,9}}));a={};assert(s.merge(a).empty());
}
