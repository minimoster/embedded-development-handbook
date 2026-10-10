#pragma once
#include "../support.hpp"

namespace hot100::p0416 {
class Solution {
public:
    bool canPartition(std::vector<int>& nums) {
        long long sum = std::accumulate(nums.begin(), nums.end(), 0LL);
        if (sum % 2) return false;
        int target = static_cast<int>(sum / 2);
        std::vector<unsigned char> possible(target + 1, 0); possible[0] = 1;
        for (int value : nums)
            for (int total = target; total >= value; --total)
                possible[total] = possible[total] || possible[total-value];
        return possible[target] != 0;
    }
};
} // namespace hot100::p0416
