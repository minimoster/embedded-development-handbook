#pragma once
#include "../support.hpp"

namespace hot100::p0226 {
class Solution {
public:
    TreeNode* invertTree(TreeNode* root) {
        if (root == nullptr) return nullptr;
        std::swap(root->left, root->right);
        invertTree(root->left);
        invertTree(root->right);
        return root;
    }
};
} // namespace hot100::p0226
