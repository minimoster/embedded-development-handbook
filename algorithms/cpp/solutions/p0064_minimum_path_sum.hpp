#pragma once
#include "../support.hpp"

namespace hot100::p0064 {
class Solution {
public:
    int minPathSum(std::vector<std::vector<int>>& grid) {
        if (grid.empty() || grid.front().empty()) return 0;
        std::size_t cols = grid.front().size();
        const long long infinity = std::numeric_limits<long long>::max() / 4;
        std::vector<long long> cost(cols, infinity); cost[0] = 0;
        for (const auto& row : grid) {
            for (std::size_t col = 0; col < cols; ++col) {
                long long left = col ? cost[col-1] : infinity;
                cost[col] = std::min(cost[col], left) + row[col];
            }
        }
        return static_cast<int>(cost.back());
    }
};
} // namespace hot100::p0064
