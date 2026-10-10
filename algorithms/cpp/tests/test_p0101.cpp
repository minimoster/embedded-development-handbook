#include "../solutions/p0101_symmetric_tree.hpp"

TreeNode* randomTree(testing::TreeArena& arena, std::mt19937& rng, int count) {
    if (count == 0) return nullptr;
    TreeNode* node = arena.make(static_cast<int>(rng() % 19) - 9);
    const int leftCount = static_cast<int>(rng() % static_cast<unsigned>(count));
    node->left = randomTree(arena, rng, leftCount);
    node->right = randomTree(arena, rng, count - 1 - leftCount);
    return node;
}

bool reference(TreeNode* root) {
    std::queue<std::pair<TreeNode*,TreeNode*>> q;
    q.emplace(root,root);
    while(!q.empty()) {
        const auto pair=q.front(); q.pop();
        TreeNode* a=pair.first; TreeNode* b=pair.second;
        if(!a || !b) { if(a!=b) return false; continue; }
        if(a->val!=b->val) return false;
        q.emplace(a->left,b->right); q.emplace(a->right,b->left);
    }
    return true;
}
int main() {
    hot100::p0101::Solution solution;
    assert(solution.isSymmetric(nullptr));
    testing::TreeArena arena;
    TreeNode* root=arena.make(5); root->left=arena.make(2); root->right=arena.make(2);
    root->left->right=arena.make(4); root->right->right=arena.make(4);
    assert(!solution.isSymmetric(root));
    root->right->left=root->right->right; root->right->right=nullptr;
    assert(solution.isSymmetric(root));
    assert(solution.isSymmetric(arena.make(-3)));
    std::mt19937 rng(101);
    for(int round=0;round<120;++round) {
        testing::TreeArena owned;
        TreeNode* tree=randomTree(owned,rng,static_cast<int>(rng()%40));
        assert(solution.isSymmetric(tree)==reference(tree));
    }
}
