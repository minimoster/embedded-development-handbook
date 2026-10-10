#pragma once
#include "../support.hpp"

namespace hot100::p0055 {
class Solution {
public:
    bool canJump(std::vector<int>& nums) {
        if (nums.empty()) return false;
        long long farthest = 0;
        for (int i = 0; i < static_cast<int>(nums.size()); ++i) {
            if (i > farthest) return false;
            farthest = std::max(farthest, static_cast<long long>(i) + nums[i]);
            if (farthest >= static_cast<long long>(nums.size()) - 1) return true;
        }
        return false;
    }
};
} // namespace hot100::p0055
