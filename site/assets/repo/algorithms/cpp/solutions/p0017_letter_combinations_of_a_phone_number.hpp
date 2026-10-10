#pragma once
#include "../support.hpp"

namespace hot100::p0017 {
class Solution {
public:
    std::vector<std::string> letterCombinations(std::string digits) {
        if (digits.empty()) return {};
        const std::array<std::string, 10> letters{"", "", "abc", "def", "ghi", "jkl", "mno", "pqrs", "tuv", "wxyz"};
        std::vector<std::string> answer;
        std::string path;
        std::function<void(std::size_t)> visit = [&](std::size_t index) {
            if (index == digits.size()) { answer.push_back(path); return; }
            const char digit = digits[index];
            if (digit < '2' || digit > '9') return;
            for (char c : letters[digit - '0']) {
                path.push_back(c); visit(index + 1); path.pop_back();
            }
        };
        visit(0); return answer;
    }
};
} // namespace hot100::p0017
