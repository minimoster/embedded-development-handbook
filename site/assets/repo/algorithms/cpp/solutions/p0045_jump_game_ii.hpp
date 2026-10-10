#pragma once
#include "../support.hpp"

namespace hot100::p0045 {
class Solution {
public:
    int jump(std::vector<int>& nums) {
        int n = static_cast<int>(nums.size());
        if (n <= 1) return 0;
        long long layerEnd = 0, nextEnd = 0;
        int steps = 0;
        for (int i = 0; i < n - 1; ++i) {
            if (i > nextEnd) return -1;
            nextEnd = std::max(nextEnd, static_cast<long long>(i) + nums[i]);
            if (i == layerEnd) {
                if (nextEnd == layerEnd) return -1;
                ++steps; layerEnd = nextEnd;
                if (layerEnd >= n - 1) return steps;
            }
        }
        return -1;
    }
};
} // namespace hot100::p0045
