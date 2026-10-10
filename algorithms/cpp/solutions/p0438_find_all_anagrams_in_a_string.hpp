#pragma once
#include "../support.hpp"

namespace hot100::p0438 {
class Solution {
public:
    std::vector<int> findAnagrams(std::string s, std::string p) {
        std::vector<int> answer;
        if (p.empty() || p.size() > s.size()) return answer;
        std::array<int, 26> need{}, window{};
        for (char c : p) ++need[c - 'a'];
        for (std::size_t i = 0; i < s.size(); ++i) {
            ++window[s[i] - 'a'];
            if (i >= p.size()) --window[s[i - p.size()] - 'a'];
            if (i + 1 >= p.size() && window == need)
                answer.push_back(static_cast<int>(i + 1 - p.size()));
        }
        return answer;
    }
};
} // namespace hot100::p0438
