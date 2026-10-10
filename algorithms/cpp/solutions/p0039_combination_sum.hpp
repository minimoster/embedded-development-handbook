#pragma once
#include "../support.hpp"

namespace hot100::p0039 {
class Solution {
public:
    std::vector<std::vector<int>> combinationSum(std::vector<int>& candidates, int target) {
        std::sort(candidates.begin(), candidates.end());
        std::vector<std::vector<int>> answer;
        std::vector<int> path;
        std::function<void(std::size_t,int)> visit = [&](std::size_t start, int remaining) {
            if (remaining == 0) { answer.push_back(path); return; }
            for (std::size_t i = start; i < candidates.size(); ++i) {
                if (candidates[i] > remaining) break;
                path.push_back(candidates[i]);
                visit(i, remaining - candidates[i]);
                path.pop_back();
            }
        };
        if (target >= 0) visit(0, target);
        return answer;
    }
};
} // namespace hot100::p0039
