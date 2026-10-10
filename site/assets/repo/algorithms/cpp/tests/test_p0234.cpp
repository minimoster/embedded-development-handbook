#include "../solutions/p0234_palindrome_linked_list.hpp"

int main() {
    hot100::p0234::Solution s;
    assert(s.isPalindrome(nullptr));
    std::mt19937 rng(234);
    for (int round=0;round<400;++round) {
        testing::ListArena arena; std::vector<ListNode*> nodes; std::vector<int> values;
        int n=static_cast<int>(rng()%45);
        for(int i=0;i<n;++i) { values.push_back(static_cast<int>(rng()%4)); nodes.push_back(arena.make(values.back())); }
        for(int i=1;i<n;++i) nodes[i-1]->next=nodes[i];
        bool expected=true; for(int i=0;i<n/2;++i) if(values[i]!=values[n-1-i]) expected=false;
        auto* head=n?nodes[0]:nullptr;
        assert(s.isPalindrome(head)==expected);
        assert(testing::listValues(head)==values);
        for(int i=0;i<n;++i) assert(nodes[i]->next==(i+1<n?nodes[i+1]:nullptr));
    }
    testing::ListArena arena; auto* a=arena.make(1); a->next=arena.make(2);
    assert(!s.isPalindrome(a)); a->next->val=1; assert(s.isPalindrome(a));
}
