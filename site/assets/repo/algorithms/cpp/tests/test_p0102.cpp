#include "../solutions/p0102_binary_tree_level_order_traversal.hpp"

TreeNode* randomTree(testing::TreeArena& arena, std::mt19937& rng, int count) {
    if (count == 0) return nullptr;
    TreeNode* node = arena.make(static_cast<int>(rng() % 19) - 9);
    const int leftCount = static_cast<int>(rng() % static_cast<unsigned>(count));
    node->left = randomTree(arena, rng, leftCount);
    node->right = randomTree(arena, rng, count - 1 - leftCount);
    return node;
}

void dfsLevels(TreeNode* node, std::size_t depth, std::vector<std::vector<int>>& out) {
    if(!node) return;
    if(out.size()==depth) out.emplace_back();
    out[depth].push_back(node->val);
    dfsLevels(node->left,depth+1,out); dfsLevels(node->right,depth+1,out);
}
int main() {
    hot100::p0102::Solution solution;
    assert(solution.levelOrder(nullptr).empty());
    testing::TreeArena arena;
    TreeNode* root=arena.make(4); root->left=arena.make(1); root->right=arena.make(7);
    root->left->right=arena.make(2); root->right->left=arena.make(6);
    assert((solution.levelOrder(root)==std::vector<std::vector<int>>{{4},{1,7},{2,6}}));
    assert((solution.levelOrder(arena.make(-1))==std::vector<std::vector<int>>{{-1}}));
    std::mt19937 rng(102);
    for(int round=0;round<120;++round) {
        testing::TreeArena owned;
        TreeNode* tree=randomTree(owned,rng,static_cast<int>(rng()%40));
        std::vector<std::vector<int>> expected; dfsLevels(tree,0,expected);
        assert(solution.levelOrder(tree)==expected);
    }
}
