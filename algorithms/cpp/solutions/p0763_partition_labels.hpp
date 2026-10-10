#pragma once
#include "../support.hpp"

namespace hot100::p0763 {
class Solution {
public:
    std::vector<int> partitionLabels(std::string s) {
        std::array<int,26> last; last.fill(-1);
        for (int i = 0; i < static_cast<int>(s.size()); ++i) last[s[i]-'a'] = i;
        std::vector<int> answer;
        int start = 0, end = 0;
        for (int i = 0; i < static_cast<int>(s.size()); ++i) {
            end = std::max(end, last[s[i]-'a']);
            if (i == end) { answer.push_back(end-start+1); start = i+1; }
        }
        return answer;
    }
};
} // namespace hot100::p0763
