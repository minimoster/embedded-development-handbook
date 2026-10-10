#pragma once
#include "../support.hpp"

namespace hot100::p0074 {
class Solution {
public:
    bool searchMatrix(std::vector<std::vector<int>>& matrix, int target) {
        if (matrix.empty() || matrix.front().empty()) return false;
        std::size_t cols = matrix.front().size();
        std::size_t lo = 0, hi = matrix.size() * cols;
        while (lo < hi) {
            std::size_t mid = lo + (hi - lo) / 2;
            int value = matrix[mid / cols][mid % cols];
            if (value < target) lo = mid + 1;
            else hi = mid;
        }
        return lo < matrix.size() * cols && matrix[lo / cols][lo % cols] == target;
    }
};
} // namespace hot100::p0074
