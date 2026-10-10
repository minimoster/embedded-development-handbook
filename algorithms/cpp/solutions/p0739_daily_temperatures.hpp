#pragma once
#include "../support.hpp"

namespace hot100::p0739 {
class Solution {
public:
    std::vector<int> dailyTemperatures(std::vector<int>& temperatures) {
        int n = static_cast<int>(temperatures.size());
        std::vector<int> answer(n, 0), pending;
        for (int i = 0; i < n; ++i) {
            while (!pending.empty() && temperatures[pending.back()] < temperatures[i]) {
                int j = pending.back(); pending.pop_back();
                answer[j] = i - j;
            }
            pending.push_back(i);
        }
        return answer;
    }
};
} // namespace hot100::p0739
