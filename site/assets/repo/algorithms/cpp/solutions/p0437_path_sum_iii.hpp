#pragma once
#include "../support.hpp"

namespace hot100::p0437 {
class Solution {
    static long long count(TreeNode* node, long long prefix, long long target,
                           std::unordered_map<long long, int>& frequency) {
        if (node == nullptr) return 0;
        prefix += node->val;
        long long answer = 0;
        const auto found = frequency.find(prefix - target);
        if (found != frequency.end()) answer += found->second;
        ++frequency[prefix];
        answer += count(node->left, prefix, target, frequency);
        answer += count(node->right, prefix, target, frequency);
        auto current = frequency.find(prefix);
        if (--current->second == 0) frequency.erase(current);
        return answer;
    }
public:
    int pathSum(TreeNode* root, int targetSum) {
        std::unordered_map<long long, int> frequency{{0LL, 1}};
        return static_cast<int>(count(root, 0LL, targetSum, frequency));
    }
};
} // namespace hot100::p0437
