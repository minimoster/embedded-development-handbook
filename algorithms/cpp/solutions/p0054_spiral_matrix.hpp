#pragma once
#include "../support.hpp"

namespace hot100::p0054 {
class Solution {
public:
    std::vector<int> spiralOrder(std::vector<std::vector<int>>& matrix) {
        std::vector<int> answer;
        if (matrix.empty() || matrix[0].empty()) return answer;
        int top = 0, bottom = static_cast<int>(matrix.size()) - 1;
        int left = 0, right = static_cast<int>(matrix[0].size()) - 1;
        while (top <= bottom && left <= right) {
            for (int j = left; j <= right; ++j) answer.push_back(matrix[top][j]);
            ++top;
            for (int i = top; i <= bottom; ++i) answer.push_back(matrix[i][right]);
            --right;
            if (top <= bottom) {
                for (int j = right; j >= left; --j) answer.push_back(matrix[bottom][j]);
                --bottom;
            }
            if (left <= right) {
                for (int i = bottom; i >= top; --i) answer.push_back(matrix[i][left]);
                ++left;
            }
        }
        return answer;
    }
};
} // namespace hot100::p0054
