#include "../solutions/p0148_sort_list.hpp"

int main() {
    hot100::p0148::Solution s;assert(s.sortList(nullptr)==nullptr);
    std::mt19937 rng(148);
    for(int round=0;round<400;++round){
        testing::ListArena arena;std::vector<ListNode*> nodes;int n=static_cast<int>(rng()%100);
        for(int i=0;i<n;++i) { nodes.push_back(arena.make(static_cast<int>(rng()%17)-8)); }
        for(int i=1;i<n;++i) { nodes[i-1]->next=nodes[i]; }
        auto expected=nodes;std::stable_sort(expected.begin(),expected.end(),[](ListNode*a,ListNode*b){return a->val<b->val;});
        auto*p=s.sortList(n?nodes[0]:nullptr);for(auto*x:expected){assert(p==x);p=p->next;}assert(p==nullptr);
    }
    testing::ListArena arena;ListNode dummy;auto*t=&dummy;for(int i=3000;i>=1;--i){t->next=arena.make(i);t=t->next;}
    auto*p=s.sortList(dummy.next);for(int i=1;i<=3000;++i){assert(p&&p->val==i);p=p->next;}assert(!p);
}
