#include "../solutions/p0002_add_two_numbers.hpp"

int main() {
    hot100::p0002::Solution s; std::mt19937 rng(2);
    for(int round=0;round<180;++round) {
        testing::ListArena arena;std::vector<int> x,y;
        auto build=[&](std::vector<int>& v,int n){ListNode dummy;auto*t=&dummy;for(int i=0;i<n;++i){int z=static_cast<int>(rng()%10);v.push_back(z);t->next=arena.make(z);t=t->next;}return dummy.next;};
        auto*a=build(x,1+static_cast<int>(rng()%70));auto*b=build(y,1+static_cast<int>(rng()%70));
        std::vector<int> expected;int c=0;for(std::size_t i=0;i<std::max(x.size(),y.size())||c;++i){int z=c+(i<x.size()?x[i]:0)+(i<y.size()?y[i]:0);expected.push_back(z%10);c=z/10;}
        auto* result=s.addTwoNumbers(a,b);assert(testing::listValues(result)==expected);
        assert(testing::listValues(a)==x&&testing::listValues(b)==y);
        std::unordered_set<ListNode*> originals;for(auto*p=a;p;p=p->next)originals.insert(p);for(auto*p=b;p;p=p->next)originals.insert(p);
        for(auto*p=result;p;p=p->next) { assert(!originals.count(p)); }
        testing::destroyList(result);
    }
    testing::ListArena arena;auto*z=arena.make(0);auto*r=s.addTwoNumbers(z,z);assert(testing::listValues(r)==std::vector<int>{0});testing::destroyList(r);
    auto*r2=s.addTwoNumbers(nullptr,z);assert(testing::listValues(r2)==std::vector<int>{0});testing::destroyList(r2);
}
