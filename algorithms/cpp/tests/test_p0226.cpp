#include "../solutions/p0226_invert_binary_tree.hpp"

TreeNode* randomTree(testing::TreeArena& arena, std::mt19937& rng, int count) {
    if (count == 0) return nullptr;
    TreeNode* node = arena.make(static_cast<int>(rng() % 19) - 9);
    const int leftCount = static_cast<int>(rng() % static_cast<unsigned>(count));
    node->left = randomTree(arena, rng, leftCount);
    node->right = randomTree(arena, rng, count - 1 - leftCount);
    return node;
}

void snapshot(TreeNode* root, std::vector<std::tuple<TreeNode*,TreeNode*,TreeNode*>>& out) {
    if(!root) return;
    out.emplace_back(root,root->left,root->right);
    snapshot(root->left,out); snapshot(root->right,out);
}
int main() {
    hot100::p0226::Solution solution;
    assert(solution.invertTree(nullptr)==nullptr);
    testing::TreeArena arena;
    TreeNode* root=arena.make(7); root->left=arena.make(2); root->right=arena.make(11);
    TreeNode* left=root->left; TreeNode* right=root->right;
    left->left=arena.make(1); right->left=arena.make(9);
    assert(solution.invertTree(root)==root);
    assert(root->left==right && root->right==left);
    assert(right->left==nullptr && right->right->val==9);
    assert(left->left==nullptr && left->right->val==1);
    std::mt19937 rng(226);
    for(int round=0;round<120;++round) {
        testing::TreeArena owned;
        TreeNode* tree=randomTree(owned,rng,static_cast<int>(rng()%35));
        std::vector<std::tuple<TreeNode*,TreeNode*,TreeNode*>> before;
        snapshot(tree,before);
        solution.invertTree(tree);
        for(const auto& entry:before) {
            assert(std::get<0>(entry)->left==std::get<2>(entry));
            assert(std::get<0>(entry)->right==std::get<1>(entry));
        }
        solution.invertTree(tree);
        for(const auto& entry:before) {
            assert(std::get<0>(entry)->left==std::get<1>(entry));
            assert(std::get<0>(entry)->right==std::get<2>(entry));
        }
    }
}
