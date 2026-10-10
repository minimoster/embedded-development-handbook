#pragma once
#include "../support.hpp"

namespace hot100::p0283 {
class Solution {
public:
    void moveZeroes(std::vector<int>& nums) {
        std::size_t write = 0;
        for (std::size_t read = 0; read < nums.size(); ++read) {
            if (nums[read] != 0) {
                if (write != read) std::swap(nums[write], nums[read]);
                ++write;
            }
        }
    }
};
} // namespace hot100::p0283
