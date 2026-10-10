#pragma once
#include "../support.hpp"

namespace hot100::p0287 {
class Solution {
public:
    int findDuplicate(std::vector<int>& nums) {
        int slow = nums[0], fast = nums[nums[0]];
        while (slow != fast) {
            slow = nums[slow];
            fast = nums[nums[fast]];
        }
        int entrance = 0;
        while (entrance != slow) {
            entrance = nums[entrance];
            slow = nums[slow];
        }
        return entrance;
    }
};
} // namespace hot100::p0287
