#pragma once
#include "../support.hpp"

namespace hot100::p0146 {
class LRUCache {
    using Entries = std::list<std::pair<int, int>>;
    std::size_t capacity_;
    Entries entries_;
    std::unordered_map<int, Entries::iterator> index_;
public:
    explicit LRUCache(int capacity)
        : capacity_(capacity > 0 ? static_cast<std::size_t>(capacity) : 0) {}
    LRUCache(const LRUCache&) = delete;
    LRUCache& operator=(const LRUCache&) = delete;
    LRUCache(LRUCache&&) = delete;
    LRUCache& operator=(LRUCache&&) = delete;
    int get(int key) {
        auto found = index_.find(key);
        if (found == index_.end()) return -1;
        entries_.splice(entries_.begin(), entries_, found->second);
        return found->second->second;
    }
    void put(int key, int value) {
        if (capacity_ == 0) return;
        auto found = index_.find(key);
        if (found != index_.end()) {
            found->second->second = value;
            entries_.splice(entries_.begin(), entries_, found->second);
            return;
        }
        entries_.emplace_front(key, value);
        try { index_.emplace(key, entries_.begin()); }
        catch (...) { entries_.pop_front(); throw; }
        if (entries_.size() > capacity_) {
            index_.erase(entries_.back().first);
            entries_.pop_back();
        }
    }
};
} // namespace hot100::p0146
