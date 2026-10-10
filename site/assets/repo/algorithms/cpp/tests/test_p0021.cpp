#include "../solutions/p0021_merge_two_sorted_lists.hpp"

int main() {
    hot100::p0021::Solution s; assert(s.mergeTwoLists(nullptr,nullptr)==nullptr);
    std::mt19937 rng(21);
    for(int round=0;round<160;++round) {
        testing::ListArena arena; std::vector<int> all;
        auto build=[&](int n) { std::vector<int> v; for(int i=0;i<n;++i)v.push_back(static_cast<int>(rng()%21)-10); std::sort(v.begin(),v.end()); ListNode dummy;auto* t=&dummy;for(int x:v){all.push_back(x); t->next=arena.make(x);t=t->next;}return dummy.next; };
        auto* a=build(static_cast<int>(rng()%25));auto* b=build(static_cast<int>(rng()%25));
        std::sort(all.begin(),all.end()); assert(testing::listValues(s.mergeTwoLists(a,b))==all);
    }
    testing::ListArena arena; auto* a=arena.make(3); auto* b=arena.make(3);
    assert(s.mergeTwoLists(a,b)==a);assert(a->next==b);
}
