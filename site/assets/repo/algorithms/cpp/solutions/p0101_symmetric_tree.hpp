#pragma once
#include "../support.hpp"

namespace hot100::p0101 {
class Solution {
    static bool mirror(TreeNode* a, TreeNode* b) {
        if (a == nullptr || b == nullptr) return a == b;
        if (a->val != b->val) return false;
        return mirror(a->left, b->right) && mirror(a->right, b->left);
    }
public:
    bool isSymmetric(TreeNode* root) {
        return root == nullptr || mirror(root->left, root->right);
    }
};
} // namespace hot100::p0101
