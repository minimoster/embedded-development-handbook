#pragma once
#include "../support.hpp"

namespace hot100::p0073 {
class Solution {
public:
    void setZeroes(std::vector<std::vector<int>>& matrix) {
        if (matrix.empty() || matrix[0].empty()) return;
        const int rows = static_cast<int>(matrix.size()), cols = static_cast<int>(matrix[0].size());
        bool firstRow = false, firstCol = false;
        for (int j = 0; j < cols; ++j) if (matrix[0][j] == 0) firstRow = true;
        for (int i = 0; i < rows; ++i) if (matrix[i][0] == 0) firstCol = true;
        for (int i = 1; i < rows; ++i) for (int j = 1; j < cols; ++j)
            if (matrix[i][j] == 0) matrix[i][0] = matrix[0][j] = 0;
        for (int i = 1; i < rows; ++i) for (int j = 1; j < cols; ++j)
            if (matrix[i][0] == 0 || matrix[0][j] == 0) matrix[i][j] = 0;
        if (firstRow) std::fill(matrix[0].begin(), matrix[0].end(), 0);
        if (firstCol) for (int i = 0; i < rows; ++i) matrix[i][0] = 0;
    }
};
} // namespace hot100::p0073
