#include "../solutions/p0138_copy_list_with_random_pointer.hpp"

int main() {
    hot100::p0138::Solution s; assert(s.copyRandomList(nullptr)==nullptr);
    std::mt19937 rng(138);
    for(int round=0;round<160;++round){
        testing::RandomArena arena;std::vector<Node*> nodes;std::vector<int> randomIndex;int n=1+static_cast<int>(rng()%35);
        for(int i=0;i<n;++i)nodes.push_back(arena.make(static_cast<int>(rng()%4)));
        for(int i=1;i<n;++i)nodes[i-1]->next=nodes[i];
        for(int i=0;i<n;++i){int j=static_cast<int>(rng()%static_cast<unsigned>(n+1))-1;randomIndex.push_back(j);nodes[i]->random=j<0?nullptr:nodes[j];}
        Node* result=s.copyRandomList(nodes[0]);std::vector<Node*> copies;std::unordered_set<Node*> originals(nodes.begin(),nodes.end());
        for(auto*p=result;p;p=p->next){assert(!originals.count(p));copies.push_back(p);}
        assert(copies.size()==nodes.size());
        for(int i=0;i<n;++i){assert(copies[i]->val==nodes[i]->val);assert(copies[i]->random==(randomIndex[i]<0?nullptr:copies[randomIndex[i]]));assert(nodes[i]->next==(i+1<n?nodes[i+1]:nullptr));assert(nodes[i]->random==(randomIndex[i]<0?nullptr:nodes[randomIndex[i]]));}
        testing::destroyRandomList(result);
    }
    testing::RandomArena arena;auto*a=arena.make(8);a->random=a;auto*c=s.copyRandomList(a);assert(c!=a&&c->random==c);testing::destroyRandomList(c);
}
