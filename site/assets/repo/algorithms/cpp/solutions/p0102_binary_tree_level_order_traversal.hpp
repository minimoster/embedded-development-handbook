#pragma once
#include "../support.hpp"

namespace hot100::p0102 {
class Solution {
public:
    std::vector<std::vector<int>> levelOrder(TreeNode* root) {
        std::vector<std::vector<int>> answer;
        if (root == nullptr) return answer;
        std::queue<TreeNode*> pending;
        pending.push(root);
        while (!pending.empty()) {
            const std::size_t count = pending.size();
            std::vector<int> level;
            level.reserve(count);
            for (std::size_t i = 0; i < count; ++i) {
                TreeNode* node = pending.front();
                pending.pop();
                level.push_back(node->val);
                if (node->left != nullptr) pending.push(node->left);
                if (node->right != nullptr) pending.push(node->right);
            }
            answer.push_back(std::move(level));
        }
        return answer;
    }
};
} // namespace hot100::p0102
