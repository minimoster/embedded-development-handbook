#include <array>
#include <cassert>
#include <functional>
#include <iostream>
#include <memory>
#include <queue>
#include <utility>
#include <vector>
/* Pure host fixtures. Dangerous dangling-capture variants are not executed. */
namespace {
struct Mark {
    std::vector<int>& trace;
    explicit Mark(std::vector<int>& t,int id):trace(t){trace.push_back(id);}
};
struct Ordered {
    Mark first; Mark second;
    explicit Ordered(std::vector<int>& t):first(t,1),second(t,2){}
};
struct Base { virtual ~Base()=default; virtual int f(int x=1)const{return x;} };
struct Derived:Base { int f(int x=2)const override{return 10+x;} };
struct ValueBase { virtual ~ValueBase()=default;virtual int value()const{return 1;} };
struct ValueDerived:ValueBase { int extra=7;int value()const override{return extra;} };
struct Record { int value=0; std::array<int,3> samples{}; };
}
int main(){
    std::vector<int> trace;Ordered ordered(trace);(void)ordered;assert((trace==std::vector<int>{1,2}));
    Record r;assert(r.value==0);assert((r.samples==std::array<int,3>{0,0,0}));
    Derived d;Base& b=d;assert(b.f()==11);assert(d.f()==12);
    ValueDerived derived;ValueBase sliced=derived;assert(derived.value()==7&&sliced.value()==1);
    auto owner=std::make_shared<int>(7);std::weak_ptr<int> observer=owner;
    std::function<int()> callback=[hold=owner]{return *hold;};owner.reset();
    assert(!observer.expired()&&callback()==7);callback={};assert(observer.expired());
    auto number=std::make_unique<int>(9);auto move_only=[p=std::move(number)]{return *p;};assert(!number&&move_only()==9);
    std::priority_queue<int> maximum;std::priority_queue<int,std::vector<int>,std::greater<int>> minimum;
    for(int n:{3,1,3,2}){maximum.push(n);minimum.push(n);}assert(maximum.top()==3&&minimum.top()==1);
    const std::array<int,4> expected{3,3,2,1};for(int n:expected){assert(maximum.top()==n);maximum.pop();}assert(maximum.empty());
    const std::array<int,4> ascending{1,2,3,3};for(int n:ascending){assert(minimum.top()==n);minimum.pop();}assert(minimum.empty());
    std::cout<<"C++ language: PASS (member order, initialized state, default/virtual dispatch, safe captures, adaptors)\n";
}
