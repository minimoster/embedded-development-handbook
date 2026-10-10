#include "../solutions/p0347_top_k_frequent_elements.hpp"

static void check(std::vector<int> a,int k) {
    hot100::p0347::Solution s; auto out=s.topKFrequent(a,k);
    assert(static_cast<int>(out.size())==k);
    std::map<int,int> freq; for(int x:a) ++freq[x];
    std::set<int> chosen(out.begin(),out.end()); assert(chosen.size()==out.size());
    int low=INT_MAX; for(int x:out) { assert(freq.count(x)); low=std::min(low,freq[x]); }
    for(auto [x,f]:freq) if(!chosen.count(x)) assert(f<=low);
}
int main() {
    std::vector<int> a{4,4,4,9,9,2}; hot100::p0347::Solution s;
    auto r=s.topKFrequent(a,2); std::sort(r.begin(),r.end()); assert((r==std::vector<int>{4,9}));
    check({-3},1); check({INT_MIN,INT_MAX,INT_MIN},1);
    std::mt19937 rng(347);
    for(int z=0;z<1200;++z) {
        std::vector<int> v(1+rng()%80); for(int& x:v) x=static_cast<int>(rng()%15)-7;
        std::set<int> unique(v.begin(),v.end()); for(int k=1;k<=static_cast<int>(unique.size());++k) check(v,k);
    }
}
