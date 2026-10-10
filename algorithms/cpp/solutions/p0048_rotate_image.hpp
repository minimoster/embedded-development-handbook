#pragma once
#include "../support.hpp"

namespace hot100::p0048 {
class Solution {
public:
    void rotate(std::vector<std::vector<int>>& matrix) {
        const int n = static_cast<int>(matrix.size());
        for (int i = 0; i < n; ++i)
            for (int j = i + 1; j < n; ++j) std::swap(matrix[i][j], matrix[j][i]);
        for (auto& row : matrix) std::reverse(row.begin(), row.end());
    }
};
} // namespace hot100::p0048
