#pragma once
#include "../support.hpp"

namespace hot100::p1143 {
class Solution {
public:
    int longestCommonSubsequence(std::string text1, std::string text2) {
        if (text2.size() > text1.size()) std::swap(text1, text2);
        std::vector<int> best(text2.size() + 1, 0);
        for (char a : text1) {
            int diagonal = 0;
            for (std::size_t j = 1; j <= text2.size(); ++j) {
                int above = best[j];
                if (a == text2[j-1]) best[j] = diagonal + 1;
                else best[j] = std::max(best[j], best[j-1]);
                diagonal = above;
            }
        }
        return best.back();
    }
};
} // namespace hot100::p1143
