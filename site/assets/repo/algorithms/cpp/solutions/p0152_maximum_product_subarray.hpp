#pragma once
#include "../support.hpp"

namespace hot100::p0152 {
class Solution {
public:
    int maxProduct(std::vector<int>& nums) {
        if (nums.empty()) throw std::invalid_argument("maxProduct requires data");
        long long highest = nums[0], lowest = nums[0], best = nums[0];
        for (std::size_t i = 1; i < nums.size(); ++i) {
            long long value = nums[i], fromHigh = highest * value, fromLow = lowest * value;
            highest = std::max({value, fromHigh, fromLow});
            lowest = std::min({value, fromHigh, fromLow});
            best = std::max(best, highest);
        }
        return static_cast<int>(best);
    }
};
} // namespace hot100::p0152
