#pragma once
#include "../support.hpp"

namespace hot100::p0035 {
class Solution {
public:
    int searchInsert(std::vector<int>& nums, int target) {
        int lo = 0, hi = static_cast<int>(nums.size());
        while (lo < hi) {
            int mid = lo + (hi - lo) / 2;
            if (nums[mid] < target) lo = mid + 1;
            else hi = mid;
        }
        return lo;
    }
};
} // namespace hot100::p0035
