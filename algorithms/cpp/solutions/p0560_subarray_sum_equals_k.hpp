#pragma once
#include "../support.hpp"

namespace hot100::p0560 {
class Solution {
public:
    int subarraySum(std::vector<int>& nums, int k) {
        std::unordered_map<long long, int> frequency;
        frequency[0] = 1;
        long long prefix = 0, answer = 0;
        for (int x : nums) {
            prefix += x;
            auto it = frequency.find(prefix - k);
            if (it != frequency.end()) answer += it->second;
            ++frequency[prefix];
        }
        return static_cast<int>(answer);
    }
};
} // namespace hot100::p0560
