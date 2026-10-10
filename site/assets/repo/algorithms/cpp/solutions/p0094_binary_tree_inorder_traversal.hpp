#pragma once
#include "../support.hpp"

namespace hot100::p0094 {
class Solution {
public:
    std::vector<int> inorderTraversal(TreeNode* root) {
        std::vector<int> answer;
        std::vector<TreeNode*> pending;
        TreeNode* cursor = root;
        while (cursor != nullptr || !pending.empty()) {
            while (cursor != nullptr) {
                pending.push_back(cursor);
                cursor = cursor->left;
            }
            cursor = pending.back();
            pending.pop_back();
            answer.push_back(cursor->val);
            cursor = cursor->right;
        }
        return answer;
    }
};
} // namespace hot100::p0094
