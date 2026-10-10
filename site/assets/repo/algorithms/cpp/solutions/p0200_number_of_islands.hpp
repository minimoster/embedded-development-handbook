#pragma once
#include "../support.hpp"

namespace hot100::p0200 {
class Solution {
public:
    int numIslands(std::vector<std::vector<char>>& grid) {
        if (grid.empty() || grid.front().empty()) return 0;
        int rows = static_cast<int>(grid.size());
        int cols = static_cast<int>(grid.front().size());
        constexpr int dr[4] = {-1, 1, 0, 0};
        constexpr int dc[4] = {0, 0, -1, 1};
        int islands = 0;
        std::vector<std::pair<int,int>> pending;
        for (int r = 0; r < rows; ++r) for (int c = 0; c < cols; ++c) {
            if (grid[r][c] != '1') continue;
            ++islands;
            grid[r][c] = '0';
            pending.emplace_back(r, c);
            while (!pending.empty()) {
                auto [x, y] = pending.back();
                pending.pop_back();
                for (int d = 0; d < 4; ++d) {
                    int nx = x + dr[d], ny = y + dc[d];
                    if (nx < 0 || nx >= rows || ny < 0 || ny >= cols || grid[nx][ny] != '1') continue;
                    grid[nx][ny] = '0';
                    pending.emplace_back(nx, ny);
                }
            }
        }
        return islands;
    }
};
} // namespace hot100::p0200
