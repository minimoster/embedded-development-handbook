#pragma once
#include "../support.hpp"

namespace hot100::p0004 {
class Solution {
public:
    double findMedianSortedArrays(std::vector<int>& a, std::vector<int>& b) {
        if (a.size() > b.size()) return findMedianSortedArrays(b, a);
        int m = static_cast<int>(a.size()), n = static_cast<int>(b.size());
        if (m + n == 0) throw std::invalid_argument("median requires data");
        int lo = 0, hi = m, leftCount = (m + n + 1) / 2;
        while (lo <= hi) {
            int i = lo + (hi - lo) / 2, j = leftCount - i;
            long long al = i ? a[i-1] : std::numeric_limits<long long>::lowest();
            long long ar = i < m ? a[i] : std::numeric_limits<long long>::max();
            long long bl = j ? b[j-1] : std::numeric_limits<long long>::lowest();
            long long br = j < n ? b[j] : std::numeric_limits<long long>::max();
            if (al > br) hi = i - 1;
            else if (bl > ar) lo = i + 1;
            else {
                long long left = std::max(al, bl);
                if ((m + n) % 2) return static_cast<double>(left);
                return (static_cast<double>(left) + static_cast<double>(std::min(ar, br))) / 2.0;
            }
        }
        throw std::logic_error("inputs must be sorted");
    }
};
} // namespace hot100::p0004
