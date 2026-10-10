#pragma once
#include "../support.hpp"

namespace hot100::p0139 {
class Solution {
public:
    bool wordBreak(std::string s, std::vector<std::string>& wordDict) {
        std::vector<unsigned char> possible(s.size() + 1, 0);
        possible[0] = 1;
        for (std::size_t end = 1; end <= s.size(); ++end) {
            for (const std::string& word : wordDict) {
                if (!word.empty() && word.size() <= end && possible[end-word.size()] &&
                    s.compare(end-word.size(), word.size(), word) == 0) {
                    possible[end] = 1; break;
                }
            }
        }
        return possible.back() != 0;
    }
};
} // namespace hot100::p0139
