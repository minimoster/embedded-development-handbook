#pragma once
#include "../support.hpp"

namespace hot100::p0295 {
class MedianFinder {
    std::priority_queue<int> lower;
    std::priority_queue<int,std::vector<int>,std::greater<int>> upper;
public:
    void addNum(int num) {
        lower.push(num);
        upper.push(lower.top()); lower.pop();
        if (upper.size() > lower.size()) {
            lower.push(upper.top()); upper.pop();
        }
    }
    double findMedian() {
        if (lower.empty()) throw std::out_of_range("median requires data");
        if (lower.size() > upper.size()) return lower.top();
        return (static_cast<double>(lower.top()) + static_cast<double>(upper.top())) / 2.0;
    }
};
} // namespace hot100::p0295
