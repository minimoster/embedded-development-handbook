#pragma once
#include "../support.hpp"

namespace hot100::p0032 {
class Solution {
public:
    int longestValidParentheses(std::string s) {
        int n = static_cast<int>(s.size()), best = 0;
        std::vector<int> ending(n, 0);
        for (int i = 1; i < n; ++i) {
            if (s[i] != ')') continue;
            int opening = i - 1 - ending[i-1];
            if (opening >= 0 && s[opening] == '(') {
                ending[i] = ending[i-1] + 2;
                if (opening > 0) ending[i] += ending[opening-1];
                best = std::max(best, ending[i]);
            }
        }
        return best;
    }
};
} // namespace hot100::p0032
