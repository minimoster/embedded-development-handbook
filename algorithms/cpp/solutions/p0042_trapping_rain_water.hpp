#pragma once
#include "../support.hpp"

namespace hot100::p0042 {
class Solution {
public:
    int trap(std::vector<int>& height) {
        int left = 0, right = static_cast<int>(height.size()) - 1;
        int leftMax = 0, rightMax = 0;
        long long water = 0;
        while (left <= right) {
            if (leftMax <= rightMax) {
                leftMax = std::max(leftMax, height[left]);
                water += leftMax - height[left++];
            } else {
                rightMax = std::max(rightMax, height[right]);
                water += rightMax - height[right--];
            }
        }
        return static_cast<int>(water);
    }
};
} // namespace hot100::p0042
