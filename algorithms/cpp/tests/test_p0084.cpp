#include "../solutions/p0084_largest_rectangle_in_histogram.hpp"

static int slow(const std::vector<int>& a) {
    int best=0;
    for(int i=0;i<static_cast<int>(a.size());++i) {
        int low=INT_MAX;
        for(int j=i;j<static_cast<int>(a.size());++j) { low=std::min(low,a[j]); best=std::max(best,low*(j-i+1)); }
    }
    return best;
}
int main() {
    hot100::p0084::Solution s;
    std::vector<int> a{3,1,4,4,2}; auto old=a; assert(s.largestRectangleArea(a)==8); assert(a==old);
    std::vector<int> e; assert(s.largestRectangleArea(e)==0);
    std::vector<int> z{0,0}; assert(s.largestRectangleArea(z)==0);
    std::vector<int> big(100000,10000); assert(s.largestRectangleArea(big)==1000000000);
    for(int n=0;n<=7;++n) {
        int total=1; for(int i=0;i<n;++i) total*=4;
        for(int mask=0;mask<total;++mask) { int k=mask; std::vector<int> v(n); for(int& x:v) { x=k%4;k/=4; } assert(s.largestRectangleArea(v)==slow(v)); }
    }
}
