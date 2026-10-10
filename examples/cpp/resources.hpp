#pragma once
#include <condition_variable>
#include <deque>
#include <mutex>
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
/* close stops new writes, then pop drains already queued items before false. */
template<class T> class BoundedQueue {
    static_assert(std::is_nothrow_move_constructible_v<T> &&
                  std::is_nothrow_move_assignable_v<T>,
                  "This teaching queue requires non-throwing element moves");
    std::mutex mutex_; std::condition_variable not_empty_,not_full_;
    std::deque<T> items_; std::size_t capacity_; bool closed_=false;
public:
    explicit BoundedQueue(std::size_t n):capacity_(n){if(!n)throw std::invalid_argument("zero capacity");}
    bool push(T value){
        std::unique_lock<std::mutex> lock(mutex_);
        not_full_.wait(lock,[&]{return closed_||items_.size()<capacity_;});
        if(closed_)return false;
        items_.push_back(std::move(value));not_empty_.notify_one();return true;
    }
    bool pop(T& out){
        std::unique_lock<std::mutex> lock(mutex_);
        not_empty_.wait(lock,[&]{return closed_||!items_.empty();});
        if(items_.empty())return false;
        out=std::move(items_.front());items_.pop_front();not_full_.notify_one();return true;
    }
    void close(){std::lock_guard<std::mutex> lock(mutex_);closed_=true;not_empty_.notify_all();not_full_.notify_all();}
};
