#pragma once
#include "../support.hpp"

namespace hot100::p0114 {
class Solution {
public:
    void flatten(TreeNode* root) {
        for (TreeNode* current = root; current != nullptr; current = current->right) {
            if (current->left == nullptr) continue;
            TreeNode* join = current->left;
            while (join->right != nullptr) join = join->right;
            join->right = current->right;
            current->right = current->left;
            current->left = nullptr;
        }
    }
};
} // namespace hot100::p0114
