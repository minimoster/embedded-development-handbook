#include "../solutions/p0230_kth_smallest_element_in_a_bst.hpp"

TreeNode* build(testing::TreeArena& arena,const std::vector<int>& v,std::size_t l,std::size_t r) {
    if(l==r) return nullptr;
    const std::size_t m=l+(r-l)/2;
    TreeNode* node=arena.make(v[m]);
    node->left=build(arena,v,l,m); node->right=build(arena,v,m+1,r);
    return node;
}
int main() {
    hot100::p0230::Solution solution;
    testing::TreeArena arena;
    std::vector<int> values{-5,-1,2,7,11};
    TreeNode* root=build(arena,values,0,values.size());
    assert(solution.kthSmallest(root,2)==-1);
    assert(solution.kthSmallest(root,5)==11);
    assert(solution.kthSmallest(root,1)==-5);
    bool caught=false;
    try { solution.kthSmallest(root,0); } catch(const std::invalid_argument&) { caught=true; }
    assert(caught); caught=false;
    try { solution.kthSmallest(nullptr,1); } catch(const std::out_of_range&) { caught=true; }
    assert(caught); caught=false;
    try { solution.kthSmallest(root,6); } catch(const std::out_of_range&) { caught=true; }
    assert(caught);
    std::mt19937 rng(230);
    for(int round=0;round<100;++round) {
        std::set<int> unique;
        const unsigned count=1+rng()%50;
        while(unique.size()<count) unique.insert(static_cast<int>(rng()%500)-250);
        std::vector<int> sorted(unique.begin(),unique.end());
        testing::TreeArena owned;
        TreeNode* tree=build(owned,sorted,0,sorted.size());
        for(std::size_t k=1;k<=sorted.size();++k)
            assert(solution.kthSmallest(tree,static_cast<int>(k))==sorted[k-1]);
    }
}
