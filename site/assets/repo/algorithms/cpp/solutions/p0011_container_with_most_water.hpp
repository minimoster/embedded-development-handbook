#pragma once
#include "../support.hpp"

namespace hot100::p0011 {
class Solution {
public:
    int maxArea(std::vector<int>& height) {
        int left = 0, right = static_cast<int>(height.size()) - 1;
        long long best = 0;
        while (left < right) {
            best = std::max(best, 1LL * (right - left) * std::min(height[left], height[right]));
            if (height[left] <= height[right]) ++left;
            else --right;
        }
        return static_cast<int>(best); // Fits the official result bounds.
    }
};
} // namespace hot100::p0011
