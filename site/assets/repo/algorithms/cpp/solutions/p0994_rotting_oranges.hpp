#pragma once
#include "../support.hpp"

namespace hot100::p0994 {
class Solution {
public:
    int orangesRotting(std::vector<std::vector<int>>& grid) {
        if (grid.empty() || grid.front().empty()) return 0;
        int rows = static_cast<int>(grid.size());
        int cols = static_cast<int>(grid.front().size());
        std::queue<std::pair<int,int>> frontier;
        int fresh = 0;
        for (int r = 0; r < rows; ++r) for (int c = 0; c < cols; ++c) {
            if (grid[r][c] == 2) frontier.emplace(r, c);
            else if (grid[r][c] == 1) ++fresh;
        }
        constexpr int dr[4] = {-1, 1, 0, 0};
        constexpr int dc[4] = {0, 0, -1, 1};
        int minutes = 0;
        while (fresh > 0 && !frontier.empty()) {
            std::size_t layerSize = frontier.size();
            for (std::size_t i = 0; i < layerSize; ++i) {
                auto [r, c] = frontier.front();
                frontier.pop();
                for (int d = 0; d < 4; ++d) {
                    int nr = r + dr[d], nc = c + dc[d];
                    if (nr < 0 || nr >= rows || nc < 0 || nc >= cols || grid[nr][nc] != 1) continue;
                    grid[nr][nc] = 2;
                    --fresh;
                    frontier.emplace(nr, nc);
                }
            }
            ++minutes;
        }
        return fresh == 0 ? minutes : -1;
    }
};
} // namespace hot100::p0994
