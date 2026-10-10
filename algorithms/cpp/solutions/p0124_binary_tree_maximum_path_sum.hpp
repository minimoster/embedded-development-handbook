#pragma once
#include "../support.hpp"

namespace hot100::p0124 {
class Solution {
    static long long gain(TreeNode* node, long long& best) {
        if (node == nullptr) return 0;
        const long long left = std::max(0LL, gain(node->left, best));
        const long long right = std::max(0LL, gain(node->right, best));
        const long long value = node->val;
        best = std::max(best, value + left + right);
        return value + std::max(left, right);
    }
public:
    int maxPathSum(TreeNode* root) {
        if (root == nullptr) throw std::invalid_argument("maximum path requires a node");
        long long best = std::numeric_limits<long long>::lowest();
        gain(root, best);
        return static_cast<int>(best);
    }
};
} // namespace hot100::p0124
