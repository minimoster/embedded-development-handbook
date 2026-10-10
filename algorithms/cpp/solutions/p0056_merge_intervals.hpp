#pragma once
#include "../support.hpp"

namespace hot100::p0056 {
class Solution {
public:
    std::vector<std::vector<int>> merge(std::vector<std::vector<int>>& intervals) {
        std::sort(intervals.begin(), intervals.end());
        std::vector<std::vector<int>> answer;
        for (const auto& current : intervals) {
            if (answer.empty() || current[0] > answer.back()[1]) answer.push_back(current);
            else answer.back()[1] = std::max(answer.back()[1], current[1]);
        }
        return answer;
    }
};
} // namespace hot100::p0056
