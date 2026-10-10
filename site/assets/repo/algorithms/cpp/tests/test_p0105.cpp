#include "../solutions/p0105_construct_binary_tree_from_preorder_and_inorder_traversal.hpp"

TreeNode* randomTree(testing::TreeArena& arena, std::mt19937& rng, int count) {
    if (count == 0) return nullptr;
    TreeNode* node = arena.make(static_cast<int>(rng() % 19) - 9);
    const int leftCount = static_cast<int>(rng() % static_cast<unsigned>(count));
    node->left = randomTree(arena, rng, leftCount);
    node->right = randomTree(arena, rng, count - 1 - leftCount);
    return node;
}

void traversals(TreeNode* node,std::vector<int>& pre,std::vector<int>& in) {
    if(!node) return;
    pre.push_back(node->val); traversals(node->left,pre,in);
    in.push_back(node->val); traversals(node->right,pre,in);
}
bool same(TreeNode* a,TreeNode* b) {
    if(!a || !b) return a==b;
    return a->val==b->val && same(a->left,b->left) && same(a->right,b->right);
}
void check(TreeNode* source,hot100::p0105::Solution& solution) {
    std::vector<int> pre,in; traversals(source,pre,in);
    const auto preBefore=pre,inBefore=in;
    TreeNode* rebuilt=solution.buildTree(pre,in);
    assert(same(source,rebuilt));
    assert(pre==preBefore && in==inBefore);
    testing::destroyTree(rebuilt);
}
int main() {
    hot100::p0105::Solution solution;
    check(nullptr,solution);
    testing::TreeArena arena;
    TreeNode* root=arena.make(7); root->left=arena.make(3); root->right=arena.make(11);
    root->left->left=arena.make(1); root->left->right=arena.make(5); root->right->left=arena.make(9);
    check(root,solution); check(root->left->left,solution); check(nullptr,solution);
    TreeNode* chain=arena.make(100); TreeNode* last=chain;
    for(int i=101;i<130;++i) { last->right=arena.make(i); last=last->right; }
    check(chain,solution);
    std::mt19937 rng(105);
    for(int round=0;round<120;++round) {
        testing::TreeArena owned;
        TreeNode* tree=randomTree(owned,rng,static_cast<int>(rng()%40));
        for(std::size_t i=0;i<owned.nodes.size();++i) owned.nodes[i]->val=static_cast<int>(i)-20;
        check(tree,solution);
    }
}
