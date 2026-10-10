#pragma once
#include "../support.hpp"

namespace hot100::p0347 {
class Solution {
public:
    std::vector<int> topKFrequent(std::vector<int>& nums, int k) {
        std::unordered_map<int,int> frequency;
        for (int value : nums) ++frequency[value];
        std::vector<std::vector<int>> buckets(nums.size() + 1);
        for (const auto& entry : frequency) buckets[entry.second].push_back(entry.first);
        std::vector<int> answer;
        for (int f = static_cast<int>(nums.size()); f > 0 && static_cast<int>(answer.size()) < k; --f) {
            for (int value : buckets[f]) {
                answer.push_back(value);
                if (static_cast<int>(answer.size()) == k) break;
            }
        }
        return answer;
    }
};
} // namespace hot100::p0347
