#pragma once
#include "../support.hpp"

namespace hot100::p0020 {
class Solution {
public:
    bool isValid(std::string s) {
        std::vector<char> expected;
        for (char c : s) {
            if (c == '(') expected.push_back(')');
            else if (c == '[') expected.push_back(']');
            else if (c == '{') expected.push_back('}');
            else {
                if (expected.empty() || expected.back() != c) return false;
                expected.pop_back();
            }
        }
        return expected.empty();
    }
};
} // namespace hot100::p0020
