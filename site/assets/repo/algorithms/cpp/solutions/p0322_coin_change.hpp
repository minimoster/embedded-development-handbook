#pragma once
#include "../support.hpp"

namespace hot100::p0322 {
class Solution {
public:
    int coinChange(std::vector<int>& coins, int amount) {
        std::vector<int> best(amount + 1, amount + 1);
        best[0] = 0;
        for (int total = 1; total <= amount; ++total) {
            for (int coin : coins) {
                if (coin > 0 && coin <= total)
                    best[total] = std::min(best[total], best[total-coin] + 1);
            }
        }
        return best[amount] <= amount ? best[amount] : -1;
    }
};
} // namespace hot100::p0322
