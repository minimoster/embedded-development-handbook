#pragma once
#include "../support.hpp"

namespace hot100::p0279 {
class Solution {
public:
    int numSquares(int n) {
        std::vector<int> best(n + 1, n + 1);
        best[0] = 0;
        for (int total = 1; total <= n; ++total)
            for (int root = 1; root <= total / root; ++root)
                best[total] = std::min(best[total], best[total-root*root] + 1);
        return best[n];
    }
};
} // namespace hot100::p0279
