#pragma once
#include "../support.hpp"

namespace hot100::p0022 {
class Solution {
public:
    std::vector<std::string> generateParenthesis(int n) {
        std::vector<std::string> answer;
        std::string path;
        std::function<void(int,int)> visit = [&](int opened, int closed) {
            if (closed == n) { answer.push_back(path); return; }
            if (opened < n) { path.push_back('('); visit(opened + 1, closed); path.pop_back(); }
            if (closed < opened) { path.push_back(')'); visit(opened, closed + 1); path.pop_back(); }
        };
        if (n >= 0) visit(0, 0);
        return answer;
    }
};
} // namespace hot100::p0022
