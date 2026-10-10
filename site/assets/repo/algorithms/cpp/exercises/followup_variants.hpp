#pragma once
#include "../support.hpp"

// Teaching variants of existing LC53/LC295 follow-ups, not extra problem entries.
// Preserve the official bounds: LC53 length <= 100000, |value| <= 10000;
// LC295 at most 50000 insertions. These are not unbounded numeric APIs.
namespace hot100::exercises {

struct IntervalSummary {
    long long total;
    long long prefix;  // Best nonempty prefix.
    long long suffix;  // Best nonempty suffix.
    long long best;    // Best nonempty subarray.
};

inline IntervalSummary mergeIntervals(const IntervalSummary& left,
                                      const IntervalSummary& right) {
    return {left.total + right.total,
            std::max(left.prefix, left.total + right.prefix),
            std::max(right.suffix, right.total + left.suffix),
            std::max({left.best, right.best, left.suffix + right.prefix})};
}

// Requires 0 <= first < last <= values.size(); interval is [first,last).
inline IntervalSummary summarizeInterval(const std::vector<int>& values,
                                         std::size_t first, std::size_t last) {
    if (last - first == 1) {
        const long long value = values[first];
        return {value, value, value, value};
    }
    const std::size_t middle = first + (last - first) / 2;
    return mergeIntervals(summarizeInterval(values, first, middle),
                          summarizeInterval(values, middle, last));
}

inline long long maxSubArrayDivideAndConquer(const std::vector<int>& values) {
    // Same explicit empty-input extension as the base lesson.
    if (values.empty()) return 0;
    return summarizeInterval(values, 0, values.size()).best;
}

inline std::pair<std::size_t, std::size_t> medianRanks(std::size_t size) {
    if (size == 0) throw std::out_of_range("median requires data");
    // One-based ranks, with no size+1 intermediate.
    return {(size - 1) / 2 + 1, size / 2 + 1};
}

// Counts alone can answer exactly only when both requested ranks lie in [0,100].
// A >=99%-in-band promise for every queried prefix is sufficient, but not needed:
// the rank check is the exact condition. Outlier values are deliberately absent.
class MedianBandCounts {
    std::array<std::size_t, 101> frequencies{};
    std::size_t below = 0;
    std::size_t above = 0;
    std::size_t total = 0;
public:
    void addNum(int value) {
        if (value < 0) ++below;
        else if (value > 100) ++above;
        else ++frequencies[static_cast<std::size_t>(value)];
        ++total;
    }
    std::size_t size() const { return total; }
    std::size_t belowCount() const { return below; }
    std::size_t aboveCount() const { return above; }

    bool rankIsInBand(std::size_t rank) const {
        return rank > below && rank <= total - above;
    }
    int selectBandRank(std::size_t rank) const {
        if (!rankIsInBand(rank))
            throw std::domain_error("exact outlier value is not stored");
        std::size_t remaining = rank - below;
        for (std::size_t value = 0; value < frequencies.size(); ++value) {
            if (remaining <= frequencies[value]) return static_cast<int>(value);
            remaining -= frequencies[value];
        }
        throw std::logic_error("frequency counts do not match total");
    }
    double findMedian() const {
        const auto ranks = medianRanks(total);
        return (static_cast<double>(selectBandRank(ranks.first)) +
                static_cast<double>(selectBandRank(ranks.second))) / 2.0;
    }
};

// Fully bounded input: O(1) insertion, O(101) query, O(101) storage.
class BoundedMedian101 {
    MedianBandCounts counts;
public:
    void addNum(int value) {
        if (value < 0 || value > 100)
            throw std::invalid_argument("value must be in [0,100]");
        counts.addNum(value);
    }
    double findMedian() const { return counts.findMedian(); }
};

// Exact even for early prefixes that do not yet satisfy a final-stream promise.
// std::multiset has ordered duplicates, but no logarithmic kth-item operation:
// outlier insert is O(log(o+1)), outlier rank traversal is O(o) worst-case.
class MedianWithOrderedOutliers {
    MedianBandCounts counts;
    std::multiset<int> below;
    std::multiset<int> above;

    static int selectOutlier(const std::multiset<int>& values, std::size_t rank) {
        auto position = values.begin();
        using Difference = std::multiset<int>::difference_type;
        std::advance(position, static_cast<Difference>(rank - 1));
        return *position;
    }
    int select(std::size_t rank) const {
        if (rank <= below.size()) return selectOutlier(below, rank);
        const std::size_t throughBand = counts.size() - above.size();
        if (rank > throughBand) return selectOutlier(above, rank - throughBand);
        return counts.selectBandRank(rank);
    }
public:
    void addNum(int value) {
        // Store a possible outlier before changing counters, so a failed
        // allocation does not leave the exact structure inconsistent.
        if (value < 0) below.insert(value);
        else if (value > 100) above.insert(value);
        counts.addNum(value);
    }
    double findMedian() const {
        const auto ranks = medianRanks(counts.size());
        return (static_cast<double>(select(ranks.first)) +
                static_cast<double>(select(ranks.second))) / 2.0;
    }
};
} // namespace hot100::exercises
