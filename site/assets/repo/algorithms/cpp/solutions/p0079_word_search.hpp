#pragma once
#include "../support.hpp"

namespace hot100::p0079 {
class Solution {
public:
    bool exist(std::vector<std::vector<char>>& board, std::string word) {
        if (word.empty()) return true;
        if (board.empty() || board[0].empty()) return false;
        int rows = static_cast<int>(board.size()), cols = static_cast<int>(board[0].size());
        if (word.size() > static_cast<std::size_t>(rows) * cols) return false;
        std::vector<std::vector<bool>> used(rows, std::vector<bool>(cols, false));
        const int dr[4]{1,-1,0,0}, dc[4]{0,0,1,-1};
        std::function<bool(int,int,std::size_t)> visit = [&](int r, int c, std::size_t index) {
            if (r < 0 || r >= rows || c < 0 || c >= cols || used[r][c] || board[r][c] != word[index]) return false;
            if (index + 1 == word.size()) return true;
            used[r][c] = true;
            bool found = false;
            for (int d = 0; d < 4 && !found; ++d) found = visit(r + dr[d], c + dc[d], index + 1);
            used[r][c] = false;
            return found;
        };
        for (int r = 0; r < rows; ++r) for (int c = 0; c < cols; ++c)
            if (visit(r, c, 0)) return true;
        return false;
    }
};
} // namespace hot100::p0079
