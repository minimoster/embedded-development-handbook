#pragma once
#include "../support.hpp"

namespace hot100::p0031 {
class Solution {
public:
    void nextPermutation(std::vector<int>& nums) {
        if (nums.size() < 2) return;
        int n = static_cast<int>(nums.size()), pivot = n - 2;
        while (pivot >= 0 && nums[pivot] >= nums[pivot+1]) --pivot;
        if (pivot >= 0) {
            int successor = n - 1;
            while (nums[successor] <= nums[pivot]) --successor;
            std::swap(nums[pivot], nums[successor]);
        }
        std::reverse(nums.begin() + (pivot + 1), nums.end());
    }
};
} // namespace hot100::p0031
