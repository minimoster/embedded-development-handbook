#include "../solutions/p0094_binary_tree_inorder_traversal.hpp"

void reference(TreeNode* node, std::vector<int>& out) {
    if (!node) return;
    reference(node->left, out);
    out.push_back(node->val);
    reference(node->right, out);
}

TreeNode* randomTree(testing::TreeArena& arena, std::mt19937& rng, int count) {
    if (count == 0) return nullptr;
    TreeNode* node = arena.make(static_cast<int>(rng() % 19) - 9);
    const int leftCount = static_cast<int>(rng() % static_cast<unsigned>(count));
    node->left = randomTree(arena, rng, leftCount);
    node->right = randomTree(arena, rng, count - 1 - leftCount);
    return node;
}

int main() {
    hot100::p0094::Solution solution;
    assert(solution.inorderTraversal(nullptr).empty());
    testing::TreeArena arena;
    TreeNode* root = arena.make(6);
    root->left = arena.make(2); root->right = arena.make(9);
    root->left->right = arena.make(4);
    assert((solution.inorderTraversal(root) == std::vector<int>{2,4,6,9}));
    TreeNode* single = arena.make(INT_MIN);
    assert((solution.inorderTraversal(single) == std::vector<int>{INT_MIN}));
    std::mt19937 rng(94);
    for (int round = 0; round < 120; ++round) {
        testing::TreeArena owned;
        TreeNode* tree = randomTree(owned, rng, static_cast<int>(rng()%40));
        std::vector<int> expected; reference(tree, expected);
        assert(solution.inorderTraversal(tree) == expected);
    }
}
