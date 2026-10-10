#include "../solutions/p0023_merge_k_sorted_lists.hpp"

int main() {
    hot100::p0023::Solution s;std::vector<ListNode*> empty;assert(s.mergeKLists(empty)==nullptr);
    std::mt19937 rng(23);
    for(int round=0;round<220;++round){
        testing::ListArena arena;std::vector<ListNode*> lists;std::vector<int> expected;std::unordered_set<ListNode*> originals;
        int k=static_cast<int>(rng()%12);
        for(int j=0;j<k;++j){std::vector<int> v;int n=static_cast<int>(rng()%24);for(int i=0;i<n;++i)v.push_back(static_cast<int>(rng()%81)-40);std::sort(v.begin(),v.end());ListNode dummy;auto*t=&dummy;for(int x:v){expected.push_back(x);t->next=arena.make(x);t=t->next;originals.insert(t);}lists.push_back(dummy.next);}
        std::sort(expected.begin(),expected.end());auto* result=s.mergeKLists(lists);assert(testing::listValues(result)==expected);
        std::unordered_set<ListNode*> used;for(auto*p=result;p;p=p->next){assert(originals.count(p));assert(used.insert(p).second);}assert(used.size()==originals.size());
    }
    testing::ListArena arena;auto*a=arena.make(INT_MIN);a->next=arena.make(INT_MAX);std::vector<ListNode*> lists{nullptr,a};assert(s.mergeKLists(lists)==a);
}
