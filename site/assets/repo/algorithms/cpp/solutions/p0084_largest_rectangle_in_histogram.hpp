#pragma once
#include "../support.hpp"

namespace hot100::p0084 {
class Solution {
public:
    int largestRectangleArea(std::vector<int>& heights) {
        int n = static_cast<int>(heights.size());
        std::vector<int> increasing;
        long long best = 0;
        for (int i = 0; i <= n; ++i) {
            int current = i == n ? 0 : heights[i];
            while (!increasing.empty() && heights[increasing.back()] > current) {
                int top = increasing.back(); increasing.pop_back();
                int left = increasing.empty() ? -1 : increasing.back();
                long long width = i - left - 1;
                best = std::max(best, width * heights[top]);
            }
            if (i < n) increasing.push_back(i);
        }
        return static_cast<int>(best);
    }
};
} // namespace hot100::p0084
