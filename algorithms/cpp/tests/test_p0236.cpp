#include "../solutions/p0236_lowest_common_ancestor_of_a_binary_tree.hpp"

TreeNode* randomTree(testing::TreeArena& arena, std::mt19937& rng, int count) {
    if (count == 0) return nullptr;
    TreeNode* node = arena.make(static_cast<int>(rng() % 19) - 9);
    const int leftCount = static_cast<int>(rng() % static_cast<unsigned>(count));
    node->left = randomTree(arena, rng, leftCount);
    node->right = randomTree(arena, rng, count - 1 - leftCount);
    return node;
}

void parents(TreeNode* node,TreeNode* parent,std::unordered_map<TreeNode*,TreeNode*>& out) {
    if(!node) return;
    out[node]=parent; parents(node->left,node,out); parents(node->right,node,out);
}
TreeNode* reference(TreeNode* p,TreeNode* q,const std::unordered_map<TreeNode*,TreeNode*>& parent) {
    std::unordered_set<TreeNode*> ancestors;
    for(TreeNode* node=p;node;node=parent.at(node)) ancestors.insert(node);
    for(TreeNode* node=q;node;node=parent.at(node)) if(ancestors.count(node)) return node;
    return nullptr;
}
int main() {
    hot100::p0236::Solution solution;
    assert(solution.lowestCommonAncestor(nullptr,nullptr,nullptr)==nullptr);
    testing::TreeArena arena;
    TreeNode* root=arena.make(8); root->left=arena.make(3); root->right=arena.make(10);
    root->left->left=arena.make(1); root->left->right=arena.make(6);
    root->left->right->left=arena.make(4);
    assert(solution.lowestCommonAncestor(root,root->left->left,root->left->right->left)==root->left);
    assert(solution.lowestCommonAncestor(root,root->left,root->left->right->left)==root->left);
    assert(solution.lowestCommonAncestor(root,root->left,root->right)==root);
    assert(solution.lowestCommonAncestor(root,root->right,root->right)==root->right);
    root->left->val=10; // Identity must not be confused with the equal-valued right child.
    assert(solution.lowestCommonAncestor(root,root->left,root->right)==root);
    std::mt19937 rng(236);
    for(int round=0;round<120;++round) {
        testing::TreeArena owned;
        TreeNode* tree=randomTree(owned,rng,1+static_cast<int>(rng()%35));
        std::unordered_map<TreeNode*,TreeNode*> parent; parents(tree,nullptr,parent);
        for(int query=0;query<20;++query) {
            TreeNode* p=owned.nodes[rng()%owned.nodes.size()].get();
            TreeNode* q=owned.nodes[rng()%owned.nodes.size()].get();
            assert(solution.lowestCommonAncestor(tree,p,q)==reference(p,q,parent));
        }
    }
}
