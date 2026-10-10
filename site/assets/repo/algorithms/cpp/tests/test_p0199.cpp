#include "../solutions/p0199_binary_tree_right_side_view.hpp"

TreeNode* randomTree(testing::TreeArena& arena, std::mt19937& rng, int count) {
    if (count == 0) return nullptr;
    TreeNode* node = arena.make(static_cast<int>(rng() % 19) - 9);
    const int leftCount = static_cast<int>(rng() % static_cast<unsigned>(count));
    node->left = randomTree(arena, rng, leftCount);
    node->right = randomTree(arena, rng, count - 1 - leftCount);
    return node;
}

std::vector<int> breadthRight(TreeNode* root) {
    std::vector<int> answer;
    if(!root) return answer;
    std::queue<TreeNode*> q; q.push(root);
    while(!q.empty()) {
        const std::size_t size=q.size();
        for(std::size_t i=0;i<size;++i) {
            TreeNode* node=q.front(); q.pop();
            if(i+1==size) answer.push_back(node->val);
            if(node->left) q.push(node->left);
            if(node->right) q.push(node->right);
        }
    }
    return answer;
}
int main() {
    hot100::p0199::Solution solution;
    assert(solution.rightSideView(nullptr).empty());
    testing::TreeArena arena;
    TreeNode* root=arena.make(6); root->left=arena.make(2); root->right=arena.make(9);
    root->left->right=arena.make(4);
    assert((solution.rightSideView(root)==std::vector<int>{6,9,4}));
    assert((solution.rightSideView(root->right)==std::vector<int>{9}));
    std::mt19937 rng(199);
    for(int round=0;round<120;++round) {
        testing::TreeArena owned;
        TreeNode* tree=randomTree(owned,rng,static_cast<int>(rng()%45));
        assert(solution.rightSideView(tree)==breadthRight(tree));
    }
}
