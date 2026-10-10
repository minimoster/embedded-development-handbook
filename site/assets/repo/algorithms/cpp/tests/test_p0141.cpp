#include "../solutions/p0141_linked_list_cycle.hpp"

int main() {
    hot100::p0141::Solution s; assert(!s.hasCycle(nullptr));
    for(int n=1;n<=30;++n) for(int entry=-1;entry<n;++entry) {
        testing::ListArena arena; std::vector<ListNode*> nodes;
        for(int i=0;i<n;++i) nodes.push_back(arena.make(5));
        for(int i=1;i<n;++i) nodes[i-1]->next=nodes[i];
        if(entry>=0) nodes.back()->next=nodes[entry];
        assert(s.hasCycle(nodes[0])==(entry>=0));
        std::unordered_set<ListNode*> seen; auto* p=nodes[0];
        while(p && seen.insert(p).second) p=p->next;
        assert(s.hasCycle(nodes[0])==(p!=nullptr));
    }
}
