#include "../solutions/p0114_flatten_binary_tree_to_linked_list.hpp"

TreeNode* randomTree(testing::TreeArena& arena, std::mt19937& rng, int count) {
    if (count == 0) return nullptr;
    TreeNode* node = arena.make(static_cast<int>(rng() % 19) - 9);
    const int leftCount = static_cast<int>(rng() % static_cast<unsigned>(count));
    node->left = randomTree(arena, rng, leftCount);
    node->right = randomTree(arena, rng, count - 1 - leftCount);
    return node;
}

void preorder(TreeNode* node,std::vector<TreeNode*>& out) {
    if(!node) return;
    out.push_back(node); preorder(node->left,out); preorder(node->right,out);
}
void check(TreeNode* root,hot100::p0114::Solution& solution) {
    std::vector<TreeNode*> expected; preorder(root,expected);
    solution.flatten(root);
    TreeNode* cursor=root;
    for(TreeNode* node:expected) {
        assert(cursor==node && cursor->left==nullptr);
        cursor=cursor->right;
    }
    assert(cursor==nullptr);
    solution.flatten(root);
    cursor=root;
    for(TreeNode* node:expected) { assert(cursor==node && !cursor->left); cursor=cursor->right; }
    assert(cursor==nullptr);
}
int main() {
    hot100::p0114::Solution solution;
    check(nullptr,solution);
    testing::TreeArena arena;
    TreeNode* root=arena.make(8); root->left=arena.make(3); root->right=arena.make(10);
    root->left->right=arena.make(6); root->left->right->left=arena.make(4);
    check(root,solution); check(arena.make(0),solution);
    std::mt19937 rng(114);
    for(int round=0;round<150;++round) {
        testing::TreeArena owned;
        TreeNode* tree=randomTree(owned,rng,static_cast<int>(rng()%55));
        check(tree,solution);
    }
}
