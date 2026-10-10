#pragma once
#include "../support.hpp"

namespace hot100::p0169 {
class Solution {
public:
    int majorityElement(std::vector<int>& nums) {
        if (nums.empty()) throw std::invalid_argument("majority requires data");
        int candidate = 0, votes = 0;
        for (int value : nums) {
            if (votes == 0) candidate = value;
            votes += value == candidate ? 1 : -1;
        }
        return candidate;
    }
};
} // namespace hot100::p0169
