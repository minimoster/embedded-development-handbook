#pragma once
#include "../support.hpp"

namespace hot100::p0098 {
class Solution {
public:
    bool isValidBST(TreeNode* root) {
        std::vector<TreeNode*> pending;
        std::optional<int> previous;
        TreeNode* cursor = root;
        while (cursor != nullptr || !pending.empty()) {
            while (cursor != nullptr) {
                pending.push_back(cursor);
                cursor = cursor->left;
            }
            cursor = pending.back();
            pending.pop_back();
            if (previous.has_value() && cursor->val <= *previous) return false;
            previous = cursor->val;
            cursor = cursor->right;
        }
        return true;
    }
};
} // namespace hot100::p0098
