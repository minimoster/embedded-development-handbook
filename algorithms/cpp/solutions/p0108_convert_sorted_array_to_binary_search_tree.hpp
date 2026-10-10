#pragma once
#include "../support.hpp"

namespace hot100::p0108 {
class Solution {
    static TreeNode* build(const std::vector<int>& values, std::size_t begin, std::size_t end) {
        if (begin == end) return nullptr;
        const std::size_t middle = begin + (end - begin) / 2;
        TreeNode* root = new TreeNode(values[middle]);
        root->left = build(values, begin, middle);
        root->right = build(values, middle + 1, end);
        return root;
    }
public:
    TreeNode* sortedArrayToBST(std::vector<int>& nums) {
        return build(nums, 0, nums.size());
    }
};
} // namespace hot100::p0108
