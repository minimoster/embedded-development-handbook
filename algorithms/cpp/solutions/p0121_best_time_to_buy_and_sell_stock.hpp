#pragma once
#include "../support.hpp"

namespace hot100::p0121 {
class Solution {
public:
    int maxProfit(std::vector<int>& prices) {
        long long lowest = std::numeric_limits<long long>::max(), best = 0;
        for (int price : prices) {
            lowest = std::min(lowest, static_cast<long long>(price));
            best = std::max(best, static_cast<long long>(price) - lowest);
        }
        return static_cast<int>(best);
    }
};
} // namespace hot100::p0121
