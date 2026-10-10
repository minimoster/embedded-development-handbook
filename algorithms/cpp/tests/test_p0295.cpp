#include "../solutions/p0295_find_median_from_data_stream.hpp"

int main() {
    hot100::p0295::MedianFinder s;
    s.addNum(8); assert(s.findMedian()==8); s.addNum(2); assert(s.findMedian()==5);
    s.addNum(11); assert(s.findMedian()==8); s.addNum(-1); assert(s.findMedian()==5);
    hot100::p0295::MedianFinder extreme; extreme.addNum(INT_MAX); extreme.addNum(INT_MAX); assert(extreme.findMedian()==INT_MAX);
    std::mt19937 rng(295); hot100::p0295::MedianFinder random; std::vector<int> ref;
    for(int i=0;i<3000;++i) {
        int x=static_cast<int>(rng()%2001)-1000; random.addNum(x); ref.push_back(x); std::sort(ref.begin(),ref.end());
        int n=static_cast<int>(ref.size()); double want=n%2 ? ref[n/2] : (static_cast<double>(ref[n/2-1])+ref[n/2])/2;
        assert(random.findMedian()==want);
    }
    hot100::p0295::MedianFinder empty; bool caught=false;
    try { empty.findMedian(); } catch(const std::out_of_range&) { caught=true; } assert(caught);
}
