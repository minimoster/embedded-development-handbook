#pragma once
#include "../support.hpp"

namespace hot100::p0062 {
class Solution {
public:
    int uniquePaths(int m, int n) {
        if (m <= 0 || n <= 0) return 0;
        if (n > m) std::swap(m, n);
        std::vector<long long> ways(n, 1);
        for (int row = 1; row < m; ++row)
            for (int col = 1; col < n; ++col)
                ways[col] += ways[col-1];
        return static_cast<int>(ways.back());
    }
};
} // namespace hot100::p0062
