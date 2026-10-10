#include "../solutions/p0004_median_of_two_sorted_arrays.hpp"

static double reference(std::vector<int> a, const std::vector<int>& b) {
    a.insert(a.end(),b.begin(),b.end()); std::sort(a.begin(),a.end());
    int n=static_cast<int>(a.size());
    return n%2 ? a[n/2] : (static_cast<double>(a[n/2-1])+a[n/2])/2;
}
int main() {
    hot100::p0004::Solution s;
    std::vector<int> a{1,7},b{2,3,9}; assert(s.findMedianSortedArrays(a,b)==3);
    std::vector<int> x{INT_MAX},y{INT_MAX}; assert(s.findMedianSortedArrays(x,y)==INT_MAX);
    std::vector<int> empty,z{INT_MIN,INT_MAX}; assert(s.findMedianSortedArrays(empty,z)==-0.5);
    std::mt19937 rng(4);
    for(int k=0;k<2000;++k) {
        std::vector<int> p(rng()%18),q(rng()%18); if(p.empty()&&q.empty()) q.push_back(0);
        for(int& v:p) v=static_cast<int>(rng()%101)-50;
        for(int& v:q) v=static_cast<int>(rng()%101)-50;
        std::sort(p.begin(),p.end()); std::sort(q.begin(),q.end());
        assert(s.findMedianSortedArrays(p,q)==reference(p,q));
    }
}
