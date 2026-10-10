#pragma once
#include "../support.hpp"

namespace hot100::p0053 {
class Solution {
public:
    int maxSubArray(std::vector<int>& nums) {
        if (nums.empty()) return 0;
        long long ending = nums[0], best = nums[0];
        for (std::size_t i = 1; i < nums.size(); ++i) {
            ending = std::max(static_cast<long long>(nums[i]), ending + nums[i]);
            best = std::max(best, ending);
        }
        return static_cast<int>(best);
    }
};
} // namespace hot100::p0053
