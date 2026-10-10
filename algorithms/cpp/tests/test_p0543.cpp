#include "../solutions/p0543_diameter_of_binary_tree.hpp"

TreeNode* randomTree(testing::TreeArena& arena, std::mt19937& rng, int count) {
    if (count == 0) return nullptr;
    TreeNode* node = arena.make(static_cast<int>(rng() % 19) - 9);
    const int leftCount = static_cast<int>(rng() % static_cast<unsigned>(count));
    node->left = randomTree(arena, rng, leftCount);
    node->right = randomTree(arena, rng, count - 1 - leftCount);
    return node;
}

int plainHeight(TreeNode* node) {
    return node ? 1+std::max(plainHeight(node->left),plainHeight(node->right)) : 0;
}
int brute(TreeNode* node) {
    if(!node) return 0;
    return std::max({plainHeight(node->left)+plainHeight(node->right),brute(node->left),brute(node->right)});
}
int main() {
    hot100::p0543::Solution solution;
    assert(solution.diameterOfBinaryTree(nullptr)==0);
    testing::TreeArena arena;
    TreeNode* root=arena.make(1); root->left=arena.make(2);
    root->left->left=arena.make(3); root->left->left->left=arena.make(4);
    root->left->right=arena.make(5); root->left->right->right=arena.make(6);
    assert(solution.diameterOfBinaryTree(root)==4);
    assert(solution.diameterOfBinaryTree(root->left->left->left)==0);
    assert(solution.diameterOfBinaryTree(nullptr)==0);
    std::mt19937 rng(543);
    for(int round=0;round<120;++round) {
        testing::TreeArena owned;
        TreeNode* tree=randomTree(owned,rng,static_cast<int>(rng()%35));
        assert(solution.diameterOfBinaryTree(tree)==brute(tree));
    }
}
