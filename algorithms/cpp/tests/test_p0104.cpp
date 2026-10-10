#include "../solutions/p0104_maximum_depth_of_binary_tree.hpp"

TreeNode* randomTree(testing::TreeArena& arena, std::mt19937& rng, int count) {
    if (count == 0) return nullptr;
    TreeNode* node = arena.make(static_cast<int>(rng() % 19) - 9);
    const int leftCount = static_cast<int>(rng() % static_cast<unsigned>(count));
    node->left = randomTree(arena, rng, leftCount);
    node->right = randomTree(arena, rng, count - 1 - leftCount);
    return node;
}

int breadthDepth(TreeNode* root) {
    if (!root) return 0;
    std::queue<TreeNode*> q; q.push(root);
    int depth = 0;
    while (!q.empty()) {
        const std::size_t size = q.size();
        ++depth;
        for (std::size_t i=0; i<size; ++i) {
            TreeNode* node=q.front(); q.pop();
            if(node->left) q.push(node->left);
            if(node->right) q.push(node->right);
        }
    }
    return depth;
}
int main() {
    hot100::p0104::Solution solution;
    assert(solution.maxDepth(nullptr) == 0);
    testing::TreeArena arena;
    TreeNode* root=arena.make(8);
    root->left=arena.make(3); root->right=arena.make(10);
    root->right->right=arena.make(12);
    assert(solution.maxDepth(root)==3);
    assert(solution.maxDepth(root->left)==1);
    assert(solution.maxDepth(nullptr)==0);
    TreeNode* chain=arena.make(0); TreeNode* tail=chain;
    for(int i=1; i<300; ++i) { tail->left=arena.make(i); tail=tail->left; }
    assert(solution.maxDepth(chain)==300);
    std::mt19937 rng(104);
    for(int round=0; round<120; ++round) {
        testing::TreeArena owned;
        TreeNode* tree=randomTree(owned,rng,static_cast<int>(rng()%45));
        assert(solution.maxDepth(tree)==breadthDepth(tree));
    }
}
