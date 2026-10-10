#include "../solutions/p0024_swap_nodes_in_pairs.hpp"

int main() {
    hot100::p0024::Solution s;assert(s.swapPairs(nullptr)==nullptr);
    for(int n=0;n<=60;++n){
        testing::ListArena arena;std::vector<ListNode*> nodes;
        for(int i=0;i<n;++i) { nodes.push_back(arena.make(1)); }
        for(int i=1;i<n;++i) { nodes[i-1]->next=nodes[i]; }
        auto*head=n?nodes[0]:nullptr;auto*result=s.swapPairs(head);
        std::vector<ListNode*> order=nodes;for(int i=1;i<n;i+=2)std::swap(order[i-1],order[i]);
        auto*p=result;for(auto*expected:order){assert(p==expected);p=p->next;}assert(p==nullptr);
        auto*restored=s.swapPairs(result);assert(restored==head);for(int i=0;i<n;++i)assert(nodes[i]->next==(i+1<n?nodes[i+1]:nullptr));
    }
}
