#pragma once
#include "../support.hpp"

namespace hot100::p0015 {
class Solution {
public:
    std::vector<std::vector<int>> threeSum(std::vector<int>& nums) {
        std::sort(nums.begin(), nums.end());
        std::vector<std::vector<int>> answer;
        const int n = static_cast<int>(nums.size());
        for (int i = 0; i + 2 < n; ++i) {
            if (i && nums[i] == nums[i - 1]) continue;
            if (nums[i] > 0) break;
            int left = i + 1, right = n - 1;
            while (left < right) {
                long long sum = 0LL + nums[i] + nums[left] + nums[right];
                if (sum < 0) ++left;
                else if (sum > 0) --right;
                else {
                    answer.push_back({nums[i], nums[left], nums[right]});
                    int a = nums[left], b = nums[right];
                    while (left < right && nums[left] == a) ++left;
                    while (left < right && nums[right] == b) --right;
                }
            }
        }
        return answer;
    }
};
} // namespace hot100::p0015
