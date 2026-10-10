#pragma once
#include "../support.hpp"

namespace hot100::p0239 {
class Solution {
public:
    std::vector<int> maxSlidingWindow(std::vector<int>& nums, int k) {
        std::vector<int> answer;
        if (k <= 0 || k > static_cast<int>(nums.size())) return answer;
        std::deque<int> candidates;
        for (int i = 0; i < static_cast<int>(nums.size()); ++i) {
            while (!candidates.empty() && candidates.front() <= i - k) candidates.pop_front();
            while (!candidates.empty() && nums[candidates.back()] <= nums[i]) candidates.pop_back();
            candidates.push_back(i);
            if (i + 1 >= k) answer.push_back(nums[candidates.front()]);
        }
        return answer;
    }
};
} // namespace hot100::p0239
