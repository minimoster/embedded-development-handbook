#include "../solutions/p0019_remove_nth_node_from_end_of_list.hpp"

int main() {
    hot100::p0019::Solution s;
    assert(s.removeNthFromEnd(nullptr,1)==nullptr);
    for(int n=1;n<=25;++n)for(int k=1;k<=n;++k){
        testing::ListArena arena;std::vector<ListNode*> nodes;std::vector<int> expected;
        for(int i=0;i<n;++i){nodes.push_back(arena.make(i));expected.push_back(i);}
        for(int i=1;i<n;++i)nodes[i-1]->next=nodes[i];
        auto*removed=nodes[n-k];expected.erase(expected.begin()+n-k);
        auto*result=s.removeNthFromEnd(nodes[0],k);
        assert(testing::listValues(result)==expected);assert(removed->next==nullptr);
    }
    testing::ListArena arena;auto*a=arena.make(4);assert(s.removeNthFromEnd(a,0)==a);assert(s.removeNthFromEnd(a,2)==a);
}
