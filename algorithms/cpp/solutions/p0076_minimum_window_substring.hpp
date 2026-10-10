#pragma once
#include "../support.hpp"

namespace hot100::p0076 {
class Solution {
public:
    std::string minWindow(std::string s, std::string t) {
        if (t.empty()) return "";
        std::array<int, 256> need{};
        for (unsigned char c : t) ++need[c];
        int missing = static_cast<int>(t.size()), left = 0;
        int bestStart = 0, bestLength = INT_MAX;
        for (int right = 0; right < static_cast<int>(s.size()); ++right) {
            unsigned char c = static_cast<unsigned char>(s[right]);
            if (need[c] > 0) --missing;
            --need[c];
            while (missing == 0) {
                if (right - left + 1 < bestLength) {
                    bestLength = right - left + 1; bestStart = left;
                }
                unsigned char out = static_cast<unsigned char>(s[left++]);
                ++need[out];
                if (need[out] > 0) ++missing;
            }
        }
        return bestLength == INT_MAX ? "" : s.substr(bestStart, bestLength);
    }
};
} // namespace hot100::p0076
