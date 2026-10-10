#pragma once
#include "../support.hpp"

namespace hot100::p0046 {
class Solution {
public:
    std::vector<std::vector<int>> permute(std::vector<int>& nums) {
        std::vector<std::vector<int>> answer;
        std::vector<int> path;
        std::vector<bool> used(nums.size(), false);
        std::function<void()> visit = [&] {
            if (path.size() == nums.size()) { answer.push_back(path); return; }
            for (std::size_t i = 0; i < nums.size(); ++i) if (!used[i]) {
                used[i] = true; path.push_back(nums[i]);
                visit();
                path.pop_back(); used[i] = false;
            }
        };
        visit(); return answer;
    }
};
} // namespace hot100::p0046
