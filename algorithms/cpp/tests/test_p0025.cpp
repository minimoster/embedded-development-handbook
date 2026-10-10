#include "../solutions/p0025_reverse_nodes_in_k_group.hpp"

int main() {
    hot100::p0025::Solution s; assert(s.reverseKGroup(nullptr,3)==nullptr);
    std::mt19937 rng(25);
    for(int round=0;round<300;++round){
        testing::ListArena arena;std::vector<ListNode*> nodes;int n=static_cast<int>(rng()%65);int k=1+static_cast<int>(rng()%15);
        for(int i=0;i<n;++i) { nodes.push_back(arena.make(7)); }
        for(int i=1;i<n;++i) { nodes[i-1]->next=nodes[i]; }
        auto order=nodes;for(int i=0;i+k<=n;i+=k)std::reverse(order.begin()+i,order.begin()+i+k);
        auto*p=s.reverseKGroup(n?nodes[0]:nullptr,k);for(auto*expected:order){assert(p==expected);p=p->next;}assert(p==nullptr);
    }
}
