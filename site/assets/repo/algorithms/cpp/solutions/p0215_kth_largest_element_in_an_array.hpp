#pragma once
#include "../support.hpp"

namespace hot100::p0215 {
class Solution {
public:
    int findKthLargest(std::vector<int>& nums, int k) {
        int n = static_cast<int>(nums.size());
        if (k < 1 || k > n) throw std::invalid_argument("invalid k");
        int wanted = n - k, lo = 0, hi = n - 1;
        std::mt19937 rng(std::random_device{}());
        while (lo <= hi) {
            int pivot = nums[std::uniform_int_distribution<int>(lo, hi)(rng)];
            int less = lo, scan = lo, greater = hi;
            while (scan <= greater) {
                if (nums[scan] < pivot) std::swap(nums[less++], nums[scan++]);
                else if (nums[scan] > pivot) std::swap(nums[scan], nums[greater--]);
                else ++scan;
            }
            if (wanted < less) hi = less - 1;
            else if (wanted > greater) lo = greater + 1;
            else return pivot;
        }
        throw std::logic_error("unreachable selection state");
    }
};
} // namespace hot100::p0215
