#include "../solutions/p0160_intersection_of_two_linked_lists.hpp"

int main() {
    hot100::p0160::Solution s;
    assert(s.getIntersectionNode(nullptr, nullptr) == nullptr);
    for (int x = 0; x < 7; ++x) for (int y = 0; y < 7; ++y) {
        testing::ListArena arena;
        ListNode* common = arena.make(7);
        common->next = arena.make(11);
        ListNode* a = common; ListNode* b = common;
        for (int i = 0; i < x; ++i) { auto* p=arena.make(7); p->next=a; a=p; }
        for (int i = 0; i < y; ++i) { auto* p=arena.make(7); p->next=b; b=p; }
        assert(s.getIntersectionNode(a,b) == common);
        assert(s.getIntersectionNode(a,nullptr) == nullptr);
    }
    testing::ListArena arena;
    auto* a=arena.make(4); auto* b=arena.make(4);
    assert(s.getIntersectionNode(a,b) == nullptr);
    assert(s.getIntersectionNode(a,a) == a);
}
