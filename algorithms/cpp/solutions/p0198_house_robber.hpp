#pragma once
#include "../support.hpp"

namespace hot100::p0198 {
class Solution {
public:
    int rob(std::vector<int>& nums) {
        long long twoBack = 0, oneBack = 0;
        for (int cash : nums) {
            long long next = std::max(oneBack, twoBack + cash);
            twoBack = oneBack; oneBack = next;
        }
        return static_cast<int>(oneBack);
    }
};
} // namespace hot100::p0198
