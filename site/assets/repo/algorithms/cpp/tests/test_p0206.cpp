#include "../solutions/p0206_reverse_linked_list.hpp"

int main() {
    hot100::p0206::Solution s;
    assert(s.reverseList(nullptr) == nullptr);
    std::mt19937 rng(206);
    for (int round=0; round<300; ++round) {
        testing::ListArena arena;
        std::vector<int> values; std::vector<ListNode*> nodes;
        int n=static_cast<int>(rng()%40);
        for (int i=0;i<n;++i) { values.push_back(static_cast<int>(rng()%9)); nodes.push_back(arena.make(values.back())); }
        for (int i=1;i<n;++i) nodes[i-1]->next=nodes[i];
        auto* head=n?nodes.front():nullptr;
        auto* result=s.reverseList(head);
        std::reverse(values.begin(),values.end());
        assert(testing::listValues(result)==values);
        auto* p=result; for (auto it=nodes.rbegin();it!=nodes.rend();++it) { assert(p==*it); p=p->next; }
        assert(p==nullptr);
        auto* restored=s.reverseList(result);
        assert(restored==head);
        for (int i=0;i<n;++i) assert(nodes[i]->next==(i+1<n?nodes[i+1]:nullptr));
    }
}
