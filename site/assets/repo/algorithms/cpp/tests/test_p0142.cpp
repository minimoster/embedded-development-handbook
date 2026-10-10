#include "../solutions/p0142_linked_list_cycle_ii.hpp"

int main() {
    hot100::p0142::Solution s; assert(s.detectCycle(nullptr)==nullptr);
    for(int n=1;n<=35;++n) for(int entry=-1;entry<n;++entry) {
        testing::ListArena arena; std::vector<ListNode*> nodes;
        for(int i=0;i<n;++i) nodes.push_back(arena.make(i%2));
        for(int i=1;i<n;++i) nodes[i-1]->next=nodes[i];
        if(entry>=0) nodes.back()->next=nodes[entry];
        auto before=nodes.back()->next;
        assert(s.detectCycle(nodes[0])==(entry<0?nullptr:nodes[entry]));
        assert(nodes.back()->next==before);
    }
}
