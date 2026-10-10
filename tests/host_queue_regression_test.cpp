#include "../examples/cpp/resources.hpp"
#include <atomic>
#include <cassert>
#include <chrono>
#include <iostream>
#include <thread>

namespace {
thread_local unsigned waiter_bit=0;
struct WaitState {
    std::mutex mutex;
    std::condition_variable changed;
    unsigned push_mask=0, pop_mask=0;
    bool entered(bool pushing,unsigned mask){
        std::unique_lock<std::mutex> lock(mutex);
        return changed.wait_for(lock,std::chrono::seconds(5),[&]{
            return ((pushing?push_mask:pop_mask)&mask)==mask;
        });
    }
};
struct Observer {
    WaitState* state=nullptr;
    void before_wait(bool pushing) const noexcept {
        if(!state)return;
        std::lock_guard<std::mutex> lock(state->mutex);
        (pushing?state->push_mask:state->pop_mask)|=waiter_bit;
        state->changed.notify_all();
    }
};
// A callback is observed while the queue mutex is held. The subsequent close/pop
// takes that same mutex, so it cannot overtake the atomic unlock-and-wait step.
static void test_blocked_close(){
    WaitState empty_state;
    BoundedQueue<int,std::allocator<int>,Observer> empty(1,{},Observer{&empty_state});
    std::atomic<bool> read_ok{true};
    std::thread reader([&]{waiter_bit=1;int value=0;read_ok=empty.pop(value);});
    const bool reader_waited=empty_state.entered(false,1);
    empty.close();reader.join();assert(reader_waited&&!read_ok);
    WaitState full_state;
    BoundedQueue<int,std::allocator<int>,Observer> full(1,{},Observer{&full_state});
    assert(full.push(7));std::atomic<bool> write_ok{true};
    std::thread writer([&]{waiter_bit=1;write_ok=full.push(9);});
    const bool writer_waited=full_state.entered(true,1);
    full.close();full.close();writer.join();
    int value=0;assert(writer_waited&&!write_ok);
    assert(full.pop(value)&&value==7);assert(!full.pop(value));
}
struct AllocationState { std::atomic<unsigned> calls{0};std::atomic<bool> fail_next{false}; };
template<class T> struct FailingAllocator {
    using value_type=T;
    AllocationState* state=nullptr;
    FailingAllocator()=default;
    explicit FailingAllocator(AllocationState& s) noexcept:state(&s){}
    template<class U> FailingAllocator(const FailingAllocator<U>& other) noexcept:state(other.state){}
    T* allocate(std::size_t n){
        if(state){++state->calls;if(state->fail_next.exchange(false))throw std::bad_alloc();}
        return std::allocator<T>{}.allocate(n);
    }
    void deallocate(T* p,std::size_t n) noexcept{std::allocator<T>{}.deallocate(p,n);}
    template<class U> bool operator==(const FailingAllocator<U>& other)const noexcept{return state==other.state;}
    template<class U> bool operator!=(const FailingAllocator<U>& other)const noexcept{return !(*this==other);}
};
struct Payload { int value=0;char padding[1024]{};explicit Payload(int n=0)noexcept:value(n){} };
using Allocator=FailingAllocator<Payload>;
using Queue=BoundedQueue<Payload,Allocator,Observer>;
// Discover the tested deque's next append allocation; do not hard-code its block
// size. The large payload makes a boundary occur quickly on the supported host.
static std::size_t allocation_boundary(){
    AllocationState allocations;Queue probe(4096,Allocator{allocations});
    for(std::size_t i=0;i<4096;++i){
        const unsigned before=allocations.calls.load();assert(probe.push(Payload{0}));
        if(i>0&&allocations.calls.load()>before)return i;
    }
    throw std::runtime_error("No append allocation boundary in bounded probe");
}
static void test_allocation_failure_handoff(){
    const std::size_t capacity=allocation_boundary();
    AllocationState allocations;WaitState waiters;
    Queue queue(capacity,Allocator{allocations},Observer{&waiters});
    for(std::size_t i=0;i<capacity;++i)assert(queue.push(Payload{7}));
    std::mutex completion_mutex;std::condition_variable completed;
    unsigned finished=0,succeeded=0,failed=0;
    auto producer=[&](unsigned bit,int value){
        waiter_bit=bit;bool accepted=false,allocation_failed=false;
        try{accepted=queue.push(Payload{value});}catch(const std::bad_alloc&){allocation_failed=true;}
        {std::lock_guard<std::mutex> lock(completion_mutex);++finished;succeeded+=accepted;failed+=allocation_failed;}
        completed.notify_all();
    };
    std::thread first(producer,1,11),second(producer,2,22);
    const bool both_waited=waiters.entered(true,3);
    allocations.fail_next=true;Payload out;
    assert(queue.pop(out)&&out.value==7);
    bool both_finished;
    {std::unique_lock<std::mutex> lock(completion_mutex);
     both_finished=completed.wait_for(lock,std::chrono::seconds(5),[&]{return finished==2;});}
    // Always release and join before assertion, including a failing regression.
    queue.close();first.join();second.join();
    assert(both_waited&&both_finished&&failed==1&&succeeded==1);
    for(std::size_t i=1;i<capacity;++i)assert(queue.pop(out)&&out.value==7);
    assert(queue.pop(out)&&(out.value==11||out.value==22));assert(!queue.pop(out));
}
static void test_closed_move_ownership(){
    int closed=0;
    {BoundedQueue<Resource> queue(1);queue.close();Resource owned(1,closed);
     assert(!queue.push(std::move(owned)));assert(!owned.valid()&&closed==1);}
    assert(closed==1);
}
}
int main(){
    test_blocked_close();test_closed_move_ownership();
    for(int round=0;round<10;++round)test_allocation_failure_handoff();
    std::cout<<"Queue regression: PASS (controlled empty/full wait, 10 allocation-failure handoffs, rejected move ownership)\n";
}
