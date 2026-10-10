#pragma once
#include "../support.hpp"

namespace hot100::p0153 {
class Solution {
public:
    int findMin(std::vector<int>& nums) {
        if (nums.empty()) throw std::invalid_argument("findMin requires a nonempty array");
        int lo = 0, hi = static_cast<int>(nums.size()) - 1;
        while (lo < hi) {
            int mid = lo + (hi - lo) / 2;
            if (nums[mid] > nums[hi]) lo = mid + 1;
            else hi = mid;
        }
        return nums[lo];
    }
};
} // namespace hot100::p0153
