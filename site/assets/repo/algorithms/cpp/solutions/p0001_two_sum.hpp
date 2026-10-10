#pragma once
#include "../support.hpp"

namespace hot100::p0001 {
class Solution {
public:
    std::vector<int> twoSum(std::vector<int>& nums, int target) {
        std::unordered_map<long long, int> seen;
        for (int i = 0; i < static_cast<int>(nums.size()); ++i) {
            long long need = static_cast<long long>(target) - nums[i];
            auto found = seen.find(need);
            if (found != seen.end()) return {found->second, i};
            seen[nums[i]] = i;
        }
        return {}; // Defensive extension: official inputs guarantee one answer.
    }
};
} // namespace hot100::p0001
