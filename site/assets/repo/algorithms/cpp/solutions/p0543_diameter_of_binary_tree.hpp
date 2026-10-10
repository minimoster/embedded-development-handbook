#pragma once
#include "../support.hpp"

namespace hot100::p0543 {
class Solution {
    static int height(TreeNode* node, int& best) {
        if (node == nullptr) return 0;
        const int left = height(node->left, best);
        const int right = height(node->right, best);
        best = std::max(best, left + right);
        return 1 + std::max(left, right);
    }
public:
    int diameterOfBinaryTree(TreeNode* root) {
        int best = 0;
        height(root, best);
        return best;
    }
};
} // namespace hot100::p0543
