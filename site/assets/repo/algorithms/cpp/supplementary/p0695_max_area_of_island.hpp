#pragma once
#include "../support.hpp"

namespace hot100::p0695 {
class Solution {
public:
    int maxAreaOfIsland(std::vector<std::vector<int>>& grid) {
        if (grid.empty() || grid[0].empty()) return 0;
        const int rows = static_cast<int>(grid.size()), cols = static_cast<int>(grid[0].size());
        const int dr[]{1,-1,0,0}, dc[]{0,0,1,-1};
        int best = 0;
        for (int r = 0; r < rows; ++r) for (int c = 0; c < cols; ++c) {
            if (grid[r][c] != 1) continue;
            int area = 0;
            std::vector<std::pair<int,int>> pending{{r,c}}; grid[r][c] = 0;
            while (!pending.empty()) {
                auto [x,y] = pending.back(); pending.pop_back(); ++area;
                for (int d = 0; d < 4; ++d) {
                    int xx = x + dr[d], yy = y + dc[d];
                    if (xx >= 0 && xx < rows && yy >= 0 && yy < cols && grid[xx][yy] == 1) {
                        grid[xx][yy] = 0; pending.emplace_back(xx,yy);
                    }
                }
            }
            best = std::max(best, area);
        }
        return best;
    }
};
} // namespace hot100::p0695
