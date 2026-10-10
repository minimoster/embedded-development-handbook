#pragma once
#include "../support.hpp"

namespace hot100::p0230 {
class Solution {
public:
    int kthSmallest(TreeNode* root, int k) {
        if (k <= 0) throw std::invalid_argument("k must be positive");
        std::vector<TreeNode*> pending;
        TreeNode* cursor = root;
        while (cursor != nullptr || !pending.empty()) {
            while (cursor != nullptr) {
                pending.push_back(cursor);
                cursor = cursor->left;
            }
            cursor = pending.back();
            pending.pop_back();
            if (--k == 0) return cursor->val;
            cursor = cursor->right;
        }
        throw std::out_of_range("k exceeds the number of nodes");
    }
};
} // namespace hot100::p0230
