#pragma once
#include "../support.hpp"

namespace hot100::p0078 {
class Solution {
public:
    std::vector<std::vector<int>> subsets(std::vector<int>& nums) {
        std::vector<std::vector<int>> answer;
        std::vector<int> path;
        std::function<void(std::size_t)> visit = [&](std::size_t start) {
            answer.push_back(path);
            for (std::size_t i = start; i < nums.size(); ++i) {
                path.push_back(nums[i]); visit(i + 1); path.pop_back();
            }
        };
        visit(0); return answer;
    }
};
} // namespace hot100::p0078
