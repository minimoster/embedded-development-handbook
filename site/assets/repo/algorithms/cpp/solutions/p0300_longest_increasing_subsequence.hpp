#pragma once
#include "../support.hpp"

namespace hot100::p0300 {
class Solution {
public:
    int lengthOfLIS(std::vector<int>& nums) {
        std::vector<int> tails;
        for (int value : nums) {
            auto it = std::lower_bound(tails.begin(), tails.end(), value);
            if (it == tails.end()) tails.push_back(value);
            else *it = value;
        }
        return static_cast<int>(tails.size());
    }
};
} // namespace hot100::p0300
