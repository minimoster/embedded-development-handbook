#pragma once
#include "../support.hpp"

namespace hot100::p0131 {
class Solution {
public:
    std::vector<std::vector<std::string>> partition(std::string s) {
        const int n = static_cast<int>(s.size());
        std::vector<std::vector<bool>> palindrome(n, std::vector<bool>(n, false));
        for (int left = n - 1; left >= 0; --left)
            for (int right = left; right < n; ++right)
                palindrome[left][right] = s[left] == s[right] && (right - left < 2 || palindrome[left + 1][right - 1]);
        std::vector<std::vector<std::string>> answer;
        std::vector<std::string> path;
        std::function<void(int)> visit = [&](int start) {
            if (start == n) { answer.push_back(path); return; }
            for (int end = start; end < n; ++end) if (palindrome[start][end]) {
                path.push_back(s.substr(start, end - start + 1));
                visit(end + 1); path.pop_back();
            }
        };
        visit(0); return answer;
    }
};
} // namespace hot100::p0131
