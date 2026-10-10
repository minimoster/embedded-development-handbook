#include "../solutions/p0739_daily_temperatures.hpp"

static std::vector<int> slow(const std::vector<int>& a) {
    std::vector<int> out(a.size());
    for(int i=0;i<static_cast<int>(a.size());++i) for(int j=i+1;j<static_cast<int>(a.size());++j)
        if(a[j]>a[i]) { out[i]=j-i; break; }
    return out;
}
int main() {
    hot100::p0739::Solution s;
    std::vector<int> a{60,60,58,64,61,70}; assert((s.dailyTemperatures(a)==std::vector<int>{3,2,1,2,1,0}));
    std::vector<int> e; assert(s.dailyTemperatures(e).empty());
    std::vector<int> d{80,70,60}; assert((s.dailyTemperatures(d)==std::vector<int>{0,0,0}));
    std::mt19937 rng(739);
    for(int z=0;z<1500;++z) { std::vector<int> v(rng()%70); for(int& x:v) x=30+static_cast<int>(rng()%71); assert(s.dailyTemperatures(v)==slow(v)); }
}
