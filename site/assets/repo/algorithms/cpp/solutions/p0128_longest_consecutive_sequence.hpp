#pragma once
#include "../support.hpp"

namespace hot100::p0128 {
class Solution {
public:
    int longestConsecutive(std::vector<int>& nums) {
        std::unordered_set<int> values(nums.begin(), nums.end());
        int best = 0;
        for (int start : values) {
            if (start != INT_MIN && values.count(start - 1)) continue;
            int current = start, length = 1;
            while (current != INT_MAX && values.count(current + 1)) {
                ++current; ++length;
            }
            best = std::max(best, length);
        }
        return best;
    }
};
} // namespace hot100::p0128
