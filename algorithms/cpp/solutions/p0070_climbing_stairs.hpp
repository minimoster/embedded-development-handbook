#pragma once
#include "../support.hpp"

namespace hot100::p0070 {
class Solution {
public:
    int climbStairs(int n) {
        if (n < 0) throw std::invalid_argument("negative stairs");
        long long before = 1, current = 1;
        for (int step = 2; step <= n; ++step) {
            long long next = before + current;
            before = current; current = next;
        }
        return static_cast<int>(current);
    }
};
} // namespace hot100::p0070
