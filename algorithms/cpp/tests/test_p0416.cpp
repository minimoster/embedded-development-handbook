#include "../solutions/p0416_partition_equal_subset_sum.hpp"

static bool slow(const std::vector<int>& a) {
    int sum=std::accumulate(a.begin(),a.end(),0);if(sum%2)return false;
    for(int mask=0;mask<(1<<a.size());++mask){int x=0;for(int i=0;i<static_cast<int>(a.size());++i)if(mask&(1<<i))x+=a[i];if(x==sum/2)return true;}return false;
}
int main() {
    hot100::p0416::Solution s;
    std::vector<int> a{3,3,4,2},b{2,2,3,5},one{8};assert(s.canPartition(a));assert(!s.canPartition(b));assert(!s.canPartition(one));
    std::vector<int> odd{1,2,4};assert(!s.canPartition(odd));
    std::mt19937 rng(416);
    for(int z=0;z<1000;++z){std::vector<int> v(1+rng()%13);for(int& x:v)x=1+static_cast<int>(rng()%15);assert(s.canPartition(v)==slow(v));}
}
