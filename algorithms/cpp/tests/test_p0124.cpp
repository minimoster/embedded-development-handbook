#include "../solutions/p0124_binary_tree_maximum_path_sum.hpp"

TreeNode* randomTree(testing::TreeArena& arena, std::mt19937& rng, int count) {
    if (count == 0) return nullptr;
    TreeNode* node = arena.make(static_cast<int>(rng() % 19) - 9);
    const int leftCount = static_cast<int>(rng() % static_cast<unsigned>(count));
    node->left = randomTree(arena, rng, leftCount);
    node->right = randomTree(arena, rng, count - 1 - leftCount);
    return node;
}

using Graph=std::unordered_map<TreeNode*,std::vector<TreeNode*>>;
void graph(TreeNode* node,Graph& g) {
    if(!node) return;
    g[node];
    for(TreeNode* child:{node->left,node->right}) {
        if(!child) continue;
        g[node].push_back(child); g[child].push_back(node); graph(child,g);
    }
}
void extend(TreeNode* node,TreeNode* previous,long long sum,const Graph& g,long long& best) {
    sum+=node->val; best=std::max(best,sum);
    for(TreeNode* neighbor:g.at(node)) if(neighbor!=previous) extend(neighbor,node,sum,g,best);
}
int brute(TreeNode* root) {
    Graph g; graph(root,g);
    long long best=LLONG_MIN;
    for(const auto& entry:g) extend(entry.first,nullptr,0,g,best);
    return static_cast<int>(best);
}
int main() {
    hot100::p0124::Solution solution;
    testing::TreeArena arena;
    TreeNode* root=arena.make(-5); root->left=arena.make(2); root->right=arena.make(8);
    root->right->left=arena.make(4); root->right->right=arena.make(6);
    assert(solution.maxPathSum(root)==18);
    assert(solution.maxPathSum(arena.make(-7))==-7);
    TreeNode* negative=arena.make(-4); negative->left=arena.make(-2); negative->right=arena.make(-9);
    assert(solution.maxPathSum(negative)==-2);
    TreeNode* zero=arena.make(0); zero->left=arena.make(0); assert(solution.maxPathSum(zero)==0);
    bool caught=false;
    try { solution.maxPathSum(nullptr); } catch(const std::invalid_argument&) { caught=true; }
    assert(caught);
    std::mt19937 rng(124);
    for(int round=0;round<160;++round) {
        testing::TreeArena owned;
        TreeNode* tree=randomTree(owned,rng,1+static_cast<int>(rng()%28));
        assert(solution.maxPathSum(tree)==brute(tree));
    }
}
