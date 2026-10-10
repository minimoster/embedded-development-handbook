#pragma once
#include "../support.hpp"

namespace hot100::p0189 {
class Solution {
public:
    void rotate(std::vector<int>& nums, int k) {
        if (nums.empty()) return;
        std::size_t shift = static_cast<std::size_t>(k) % nums.size();
        std::reverse(nums.begin(), nums.end());
        std::reverse(nums.begin(), nums.begin() + static_cast<std::ptrdiff_t>(shift));
        std::reverse(nums.begin() + static_cast<std::ptrdiff_t>(shift), nums.end());
    }
};
} // namespace hot100::p0189
