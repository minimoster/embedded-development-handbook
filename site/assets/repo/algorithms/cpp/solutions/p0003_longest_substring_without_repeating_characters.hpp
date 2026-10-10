#pragma once
#include "../support.hpp"

namespace hot100::p0003 {
class Solution {
public:
    int lengthOfLongestSubstring(std::string s) {
        std::array<int, 256> last; last.fill(-1);
        int left = 0, best = 0;
        for (int right = 0; right < static_cast<int>(s.size()); ++right) {
            unsigned char c = static_cast<unsigned char>(s[right]);
            left = std::max(left, last[c] + 1);
            best = std::max(best, right - left + 1);
            last[c] = right;
        }
        return best;
    }
};
} // namespace hot100::p0003
