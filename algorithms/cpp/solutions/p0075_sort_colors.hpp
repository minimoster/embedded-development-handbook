#pragma once
#include "../support.hpp"

namespace hot100::p0075 {
class Solution {
public:
    void sortColors(std::vector<int>& nums) {
        std::size_t low = 0, scan = 0, high = nums.size();
        while (scan < high) {
            if (nums[scan] == 0) std::swap(nums[low++], nums[scan++]);
            else if (nums[scan] == 1) ++scan;
            else std::swap(nums[scan], nums[--high]);
        }
    }
};
} // namespace hot100::p0075
