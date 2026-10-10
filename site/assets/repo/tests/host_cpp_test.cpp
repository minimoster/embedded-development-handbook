#include "../examples/cpp/resources.hpp"
#include <cassert>
#include <iostream>
#include <thread>
#include <type_traits>
#include <vector>
static void test_resource(){
    static_assert(!std::is_copy_constructible_v<Resource>);
    int closed=0;
    {Resource a(1,closed);Resource b(std::move(a));assert(!a.valid()&&b.valid());
     Resource c(2,closed);c=std::move(b);assert(closed==1);Resource& alias=c;c=std::move(alias);assert(c.valid());}
    assert(closed==2);
}
static void test_queue(){
    BoundedQueue<int> q(3);std::vector<int> values;
    std::thread consumer([&]{int x=0;while(q.pop(x))values.push_back(x);});
    for(int i=0;i<10000;++i)assert(q.push(i));
    q.close();q.close();consumer.join();
    assert(values.size()==10000);for(int i=0;i<10000;++i)assert(values[static_cast<std::size_t>(i)]==i);
    assert(!q.push(7));int x=0;assert(!q.pop(x));
    BoundedQueue<int> empty(1);std::thread waiter([&]{int y=0;assert(!empty.pop(y));});empty.close();waiter.join();
    BoundedQueue<int> full(1);assert(full.push(1));
    std::thread writer([&]{assert(!full.push(2));});full.close();writer.join();assert(full.pop(x)&&x==1);assert(!full.pop(x));
}
int main(){test_resource();test_queue();std::cout<<"C++ core: PASS (move ownership, 10000 FIFO items, empty/full close)\n";}
