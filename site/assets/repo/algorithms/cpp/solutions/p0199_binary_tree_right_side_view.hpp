#pragma once
#include "../support.hpp"

namespace hot100::p0199 {
class Solution {
    static void visit(TreeNode* node, std::size_t depth, std::vector<int>& answer) {
        if (node == nullptr) return;
        if (answer.size() == depth) answer.push_back(node->val);
        visit(node->right, depth + 1, answer);
        visit(node->left, depth + 1, answer);
    }
public:
    std::vector<int> rightSideView(TreeNode* root) {
        std::vector<int> answer;
        visit(root, 0, answer);
        return answer;
    }
};
} // namespace hot100::p0199
