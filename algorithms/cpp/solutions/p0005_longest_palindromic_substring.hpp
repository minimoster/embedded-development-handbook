#pragma once
#include "../support.hpp"

namespace hot100::p0005 {
class Solution {
public:
    std::string longestPalindrome(std::string s) {
        int n = static_cast<int>(s.size());
        if (!n) return "";
        std::vector<std::vector<unsigned char>> palindrome(n, std::vector<unsigned char>(n, 0));
        int bestStart = 0, bestLength = 1;
        for (int length = 1; length <= n; ++length) {
            for (int left = 0; left + length <= n; ++left) {
                int right = left + length - 1;
                if (s[left] == s[right] && (length <= 2 || palindrome[left+1][right-1])) {
                    palindrome[left][right] = 1;
                    if (length > bestLength) { bestStart = left; bestLength = length; }
                }
            }
        }
        return s.substr(bestStart, bestLength);
    }
};
} // namespace hot100::p0005
