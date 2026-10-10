#pragma once
#include "../support.hpp"

namespace hot100::p0051 {
class Solution {
public:
    std::vector<std::vector<std::string>> solveNQueens(int n) {
        std::vector<std::vector<std::string>> answer;
        if (n <= 0) return answer;
        std::vector<std::string> board(n, std::string(n, '.'));
        std::vector<bool> columns(n, false), diagonalA(2*n-1, false), diagonalB(2*n-1, false);
        std::function<void(int)> visit = [&](int row) {
            if (row == n) { answer.push_back(board); return; }
            for (int col = 0; col < n; ++col) {
                int a = row - col + n - 1, b = row + col;
                if (columns[col] || diagonalA[a] || diagonalB[b]) continue;
                columns[col] = diagonalA[a] = diagonalB[b] = true; board[row][col] = 'Q';
                visit(row + 1);
                board[row][col] = '.'; columns[col] = diagonalA[a] = diagonalB[b] = false;
            }
        };
        visit(0); return answer;
    }
};
} // namespace hot100::p0051
