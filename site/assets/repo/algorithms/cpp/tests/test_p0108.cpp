#include "../solutions/p0108_convert_sorted_array_to_binary_search_tree.hpp"

void inorder(TreeNode* node, std::vector<int>& out) {
    if(!node) return;
    inorder(node->left,out); out.push_back(node->val); inorder(node->right,out);
}
int balancedHeight(TreeNode* node) {
    if(!node) return 0;
    const int left=balancedHeight(node->left), right=balancedHeight(node->right);
    assert(std::abs(left-right)<=1);
    return 1+std::max(left,right);
}
void check(std::vector<int> values) {
    hot100::p0108::Solution solution;
    const auto before=values;
    TreeNode* root=solution.sortedArrayToBST(values);
    std::vector<int> actual; inorder(root,actual);
    assert(actual==values && values==before);
    balancedHeight(root);
    testing::destroyTree(root);
}
int main() {
    check({}); check({4}); check({INT_MIN,0,INT_MAX}); check({-8,-3,0,5,12,20});
    std::mt19937 rng(108);
    for(int round=0;round<100;++round) {
        std::set<int> unique;
        const unsigned count=rng()%80;
        while(unique.size()<count) unique.insert(static_cast<int>(rng()%1000)-500);
        check(std::vector<int>(unique.begin(),unique.end()));
    }
}
