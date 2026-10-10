#pragma once
#include "../support.hpp"

namespace hot100::p0104 {
class Solution {
public:
    int maxDepth(TreeNode* root) {
        if (root == nullptr) return 0;
        const int left = maxDepth(root->left);
        const int right = maxDepth(root->right);
        return 1 + std::max(left, right);
    }
};
} // namespace hot100::p0104
