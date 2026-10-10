#pragma once
#include "../support.hpp"

namespace hot100::p0049 {
class Solution {
public:
    std::vector<std::vector<std::string>> groupAnagrams(std::vector<std::string>& words) {
        std::unordered_map<std::string, std::size_t> group;
        std::vector<std::vector<std::string>> answer;
        for (const auto& word : words) {
            std::string key = word;
            std::sort(key.begin(), key.end());
            auto it = group.find(key);
            if (it == group.end()) {
                std::size_t index = answer.size();
                group.emplace(key, index);
                answer.push_back({word});
            } else answer[it->second].push_back(word);
        }
        return answer;
    }
};
} // namespace hot100::p0049
