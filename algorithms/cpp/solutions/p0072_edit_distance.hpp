#pragma once
#include "../support.hpp"

namespace hot100::p0072 {
class Solution {
public:
    int minDistance(std::string word1, std::string word2) {
        if (word2.size() > word1.size()) std::swap(word1, word2);
        std::vector<int> distance(word2.size() + 1);
        std::iota(distance.begin(), distance.end(), 0);
        for (std::size_t i = 1; i <= word1.size(); ++i) {
            int diagonal = distance[0]; distance[0] = static_cast<int>(i);
            for (std::size_t j = 1; j <= word2.size(); ++j) {
                int above = distance[j];
                if (word1[i-1] == word2[j-1]) distance[j] = diagonal;
                else distance[j] = 1 + std::min({above, distance[j-1], diagonal});
                diagonal = above;
            }
        }
        return distance.back();
    }
};
} // namespace hot100::p0072
