#pragma once
#include <condition_variable>
#include <deque>
#include <mutex>
#include <memory>
#include <stdexcept>
#include <utility>
#include <type_traits>
/* Non-owning counter is a test substitute for an actual close operation. */
class Resource {
    int id_=-1; int* closed_=nullptr;
    void release() noexcept { if(id_>=0&&closed_){++*closed_;} id_=-1; }
public:
    Resource(int id,int& closed) noexcept : id_(id),closed_(&closed) {}
    ~Resource(){release();}
    Resource(const Resource&)=delete;
    Resource& operator=(const Resource&)=delete;
    Resource(Resource&& o) noexcept : id_(std::exchange(o.id_,-1)),closed_(o.closed_) {}
    Resource& operator=(Resource&& o) noexcept {
        if(this!=&o){release();id_=std::exchange(o.id_,-1);closed_=o.closed_;}return *this;
    }
    bool valid()const noexcept{return id_>=0;}
};
/* The default observer adds no behavior. Test observers must not throw/reenter:
 * before_wait runs under the queue mutex immediately before atomic unlock/wait. */
struct QueueWaitObserver { void before_wait(bool) const noexcept {} };
/* close stops new writes, then pop drains already queued items before false.
 * Join all users before destruction. No fairness, ISR or allocation-free promise. */
template<class T, class Allocator=std::allocator<T>, class WaitObserver=QueueWaitObserver>
class BoundedQueue {
    static_assert(std::is_nothrow_move_constructible_v<T> &&
                  std::is_nothrow_move_assignable_v<T>,
                  "This teaching queue requires non-throwing element moves");
    std::mutex mutex_; std::condition_variable not_empty_,not_full_;
    static_assert(noexcept(std::declval<WaitObserver&>().before_wait(true)),
                  "The wait observer must not throw");
    std::deque<T,Allocator> items_; std::size_t capacity_; bool closed_=false;
    WaitObserver observer_;
public:
    explicit BoundedQueue(std::size_t n, const Allocator& allocator=Allocator{},
                          WaitObserver observer=WaitObserver{})
        :items_(allocator),capacity_(n),observer_(std::move(observer)) {
        if(!n)throw std::invalid_argument("zero capacity");
    }
    bool push(T value){
        std::unique_lock<std::mutex> lock(mutex_);
        while(!closed_ && items_.size()==capacity_){
            observer_.before_wait(true);not_full_.wait(lock);
        }
        if(closed_)return false;
        try { items_.push_back(std::move(value)); }
        catch (...) {
            // Nothrow element moves + deque single-end insertion: no effects on
            // allocation failure. Pass the still-available slot to another writer.
            not_full_.notify_one();throw;
        }
        not_empty_.notify_one();return true;
    }
    bool pop(T& out){
        std::unique_lock<std::mutex> lock(mutex_);
        while(!closed_ && items_.empty()){
            observer_.before_wait(false);not_empty_.wait(lock);
        }
        if(items_.empty())return false;
        out=std::move(items_.front());items_.pop_front();not_full_.notify_one();return true;
    }
    void close(){std::lock_guard<std::mutex> lock(mutex_);closed_=true;not_empty_.notify_all();not_full_.notify_all();}
};
