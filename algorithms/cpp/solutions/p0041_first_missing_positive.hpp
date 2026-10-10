#pragma once
#include "../support.hpp"

namespace hot100::p0041 {
class Solution {
public:
    int firstMissingPositive(std::vector<int>& nums) {
        const int n = static_cast<int>(nums.size());
        for (int i = 0; i < n; ++i) {
            while (nums[i] >= 1 && nums[i] <= n && nums[nums[i] - 1] != nums[i]) {
                int destination = nums[i] - 1;
                std::swap(nums[i], nums[destination]);
            }
        }
        for (int i = 0; i < n; ++i) if (nums[i] != i + 1) return i + 1;
        return n + 1;
    }
};
} // namespace hot100::p0041
