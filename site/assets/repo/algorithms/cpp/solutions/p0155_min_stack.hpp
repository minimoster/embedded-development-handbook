#pragma once
#include "../support.hpp"

namespace hot100::p0155 {
class MinStack {
    std::deque<std::pair<int,int>> items;
    void requireData() const {
        if (items.empty()) throw std::out_of_range("empty MinStack");
    }
public:
    void push(int val) {
        int minimum = items.empty() ? val : std::min(val, items.back().second);
        items.emplace_back(val, minimum);
    }
    void pop() { requireData(); items.pop_back(); }
    int top() { requireData(); return items.back().first; }
    int getMin() { requireData(); return items.back().second; }
};
} // namespace hot100::p0155
