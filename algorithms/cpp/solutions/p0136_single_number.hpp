#pragma once
#include "../support.hpp"

namespace hot100::p0136 {
class Solution {
public:
    int singleNumber(std::vector<int>& nums) {
        int result = 0;
        for (int value : nums) result ^= value;
        return result;
    }
};
} // namespace hot100::p0136
