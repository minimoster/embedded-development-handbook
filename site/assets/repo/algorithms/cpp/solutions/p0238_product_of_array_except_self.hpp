#pragma once
#include "../support.hpp"

namespace hot100::p0238 {
class Solution {
public:
    std::vector<int> productExceptSelf(std::vector<int>& nums) {
        std::vector<int> answer(nums.size(), 1);
        long long prefix = 1;
        for (std::size_t i = 0; i < nums.size(); ++i) {
            answer[i] = static_cast<int>(prefix);
            prefix *= nums[i];
        }
        long long suffix = 1;
        for (int i = static_cast<int>(nums.size()) - 1; i >= 0; --i) {
            answer[i] = static_cast<int>(answer[i] * suffix);
            suffix *= nums[i];
        }
        return answer;
    }
};
} // namespace hot100::p0238
