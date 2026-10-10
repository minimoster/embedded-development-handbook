#pragma once
#include "../support.hpp"

namespace hot100::p0034 {
class Solution {
    int bound(const std::vector<int>& nums, int target, bool upper) {
        int lo = 0, hi = static_cast<int>(nums.size());
        while (lo < hi) {
            int mid = lo + (hi - lo) / 2;
            if (nums[mid] < target || (upper && nums[mid] == target)) lo = mid + 1;
            else hi = mid;
        }
        return lo;
    }
public:
    std::vector<int> searchRange(std::vector<int>& nums, int target) {
        int first = bound(nums, target, false);
        if (first == static_cast<int>(nums.size()) || nums[first] != target) return {-1,-1};
        return {first, bound(nums, target, true) - 1};
    }
};
} // namespace hot100::p0034
