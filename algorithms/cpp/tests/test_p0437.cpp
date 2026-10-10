#include "../solutions/p0437_path_sum_iii.hpp"

TreeNode* randomTree(testing::TreeArena& arena, std::mt19937& rng, int count) {
    if (count == 0) return nullptr;
    TreeNode* node = arena.make(static_cast<int>(rng() % 19) - 9);
    const int leftCount = static_cast<int>(rng() % static_cast<unsigned>(count));
    node->left = randomTree(arena, rng, leftCount);
    node->right = randomTree(arena, rng, count - 1 - leftCount);
    return node;
}

int from(TreeNode* node,long long remaining) {
    if(!node) return 0;
    remaining-=node->val;
    return (remaining==0 ? 1 : 0)+from(node->left,remaining)+from(node->right,remaining);
}
int brute(TreeNode* node,int target) {
    if(!node) return 0;
    return from(node,target)+brute(node->left,target)+brute(node->right,target);
}
int main() {
    hot100::p0437::Solution solution;
    assert(solution.pathSum(nullptr,0)==0);
    testing::TreeArena arena;
    TreeNode* root=arena.make(3); root->left=arena.make(-1); root->right=arena.make(2);
    root->left->left=arena.make(2); root->left->right=arena.make(1);
    assert(solution.pathSum(root,2)==3);
    TreeNode* zero=arena.make(0); TreeNode* tail=zero;
    for(int i=1;i<20;++i) { tail->right=arena.make(0); tail=tail->right; }
    assert(solution.pathSum(zero,0)==210);
    assert(solution.pathSum(root,2)==3);
    TreeNode* large=arena.make(1000000000); tail=large;
    for(int i=1;i<4;++i) { tail->left=arena.make(1000000000); tail=tail->left; }
    assert(solution.pathSum(large,-294967296)==0);
    assert(solution.pathSum(large,1000000000)==4);
    std::mt19937 rng(437);
    for(int round=0;round<180;++round) {
        testing::TreeArena owned;
        TreeNode* tree=randomTree(owned,rng,static_cast<int>(rng()%35));
        const int target=static_cast<int>(rng()%31)-15;
        assert(solution.pathSum(tree,target)==brute(tree,target));
    }
}
