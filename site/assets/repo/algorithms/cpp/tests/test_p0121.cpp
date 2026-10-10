#include "../solutions/p0121_best_time_to_buy_and_sell_stock.hpp"

static int slow(const std::vector<int>& a) {
    int best=0;
    for(int i=0;i<static_cast<int>(a.size());++i) for(int j=i+1;j<static_cast<int>(a.size());++j) best=std::max(best,a[j]-a[i]);
    return best;
}
int main() {
    hot100::p0121::Solution s;
    std::vector<int> a{9,6,8,3,10,4}; assert(s.maxProfit(a)==7);
    std::vector<int> d{5,4,1}; assert(s.maxProfit(d)==0);
    std::vector<int> one{7},empty; assert(s.maxProfit(one)==0); assert(s.maxProfit(empty)==0);
    std::mt19937 rng(121);
    for(int z=0;z<2000;++z) { std::vector<int> v(rng()%30); for(int& x:v) x=static_cast<int>(rng()%100); assert(s.maxProfit(v)==slow(v)); }
}
