#include "../solutions/p0098_validate_binary_search_tree.hpp"

TreeNode* randomTree(testing::TreeArena& arena, std::mt19937& rng, int count) {
    if (count == 0) return nullptr;
    TreeNode* node = arena.make(static_cast<int>(rng() % 19) - 9);
    const int leftCount = static_cast<int>(rng() % static_cast<unsigned>(count));
    node->left = randomTree(arena, rng, leftCount);
    node->right = randomTree(arena, rng, count - 1 - leftCount);
    return node;
}

bool rangeCheck(TreeNode* node,long long low,long long high) {
    if(!node) return true;
    return low<node->val && node->val<high &&
        rangeCheck(node->left,low,node->val) && rangeCheck(node->right,node->val,high);
}
int main() {
    hot100::p0098::Solution solution;
    assert(solution.isValidBST(nullptr));
    testing::TreeArena arena;
    TreeNode* root=arena.make(10); root->left=arena.make(4); root->right=arena.make(15);
    root->right->left=arena.make(6);
    assert(!solution.isValidBST(root));
    root->right->left->val=12; assert(solution.isValidBST(root));
    root->left->val=10; assert(!solution.isValidBST(root));
    TreeNode* extreme=arena.make(0); extreme->left=arena.make(INT_MIN); extreme->right=arena.make(INT_MAX);
    assert(solution.isValidBST(extreme));
    assert(solution.isValidBST(extreme->left));
    std::mt19937 rng(98);
    for(int round=0;round<150;++round) {
        testing::TreeArena owned;
        TreeNode* tree=randomTree(owned,rng,static_cast<int>(rng()%30));
        assert(solution.isValidBST(tree)==rangeCheck(tree,LLONG_MIN,LLONG_MAX));
    }
}
