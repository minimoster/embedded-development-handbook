#pragma once
#include "../support.hpp"

namespace hot100::p0118 {
class Solution {
public:
    std::vector<std::vector<int>> generate(int numRows) {
        std::vector<std::vector<int>> triangle;
        for (int row = 0; row < numRows; ++row) {
            std::vector<int> values(row + 1, 1);
            for (int col = 1; col < row; ++col)
                values[col] = triangle.back()[col-1] + triangle.back()[col];
            triangle.push_back(std::move(values));
        }
        return triangle;
    }
};
} // namespace hot100::p0118
