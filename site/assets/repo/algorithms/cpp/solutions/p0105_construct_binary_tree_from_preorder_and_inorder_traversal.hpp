#pragma once
#include "../support.hpp"

namespace hot100::p0105 {
class Solution {
    static TreeNode* build(const std::vector<int>& preorder,
                           const std::unordered_map<int, std::size_t>& position,
                           std::size_t begin, std::size_t end, std::size_t& next) {
        if (begin == end) return nullptr;
        const int value = preorder[next++];
        const std::size_t middle = position.at(value);
        TreeNode* root = new TreeNode(value);
        root->left = build(preorder, position, begin, middle, next);
        root->right = build(preorder, position, middle + 1, end, next);
        return root;
    }
public:
    TreeNode* buildTree(std::vector<int>& preorder, std::vector<int>& inorder) {
        std::unordered_map<int, std::size_t> position;
        position.reserve(inorder.size());
        for (std::size_t i = 0; i < inorder.size(); ++i) position.emplace(inorder[i], i);
        std::size_t next = 0;
        return build(preorder, position, 0, inorder.size(), next);
    }
};
} // namespace hot100::p0105
