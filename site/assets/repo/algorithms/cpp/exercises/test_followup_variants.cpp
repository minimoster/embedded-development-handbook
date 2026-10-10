#include "followup_variants.hpp"
#include "../solutions/p0053_maximum_subarray.hpp"
#include "../solutions/p0072_edit_distance.hpp"
#include "../solutions/p0076_minimum_window_substring.hpp"
#include "../solutions/p0198_house_robber.hpp"
#include "../solutions/p0295_find_median_from_data_stream.hpp"

using namespace hot100::exercises;

static IntervalSummary bruteSummary(const std::vector<int>& values,
                                   std::size_t first, std::size_t last) {
    long long total = 0, prefix = LLONG_MIN, suffix = LLONG_MIN, best = LLONG_MIN;
    for (std::size_t i = first; i < last; ++i) {
        total += values[i];
        prefix = std::max(prefix, total);
        long long sum = 0;
        for (std::size_t j = i; j < last; ++j) {
            sum += values[j];
            best = std::max(best, sum);
        }
    }
    long long sum = 0;
    for (std::size_t i = last; i > first; --i) {
        sum += values[i - 1];
        suffix = std::max(suffix, sum);
    }
    return {total, prefix, suffix, best};
}

static void assertSummary(const IntervalSummary& got, const IntervalSummary& want) {
    assert(got.total == want.total);
    assert(got.prefix == want.prefix);
    assert(got.suffix == want.suffix);
    assert(got.best == want.best);
}

static void testDivideAndConquer() {
    assert(maxSubArrayDivideAndConquer({}) == 0);
    assert(maxSubArrayDivideAndConquer({-8, -3, -5}) == -3);
    const std::vector<int> worked{-4, 5, -2, 3, -6};
    assertSummary(summarizeInterval(worked, 0, 2), {1, 1, 5, 5});
    assertSummary(summarizeInterval(worked, 2, 5), {-5, 1, -3, 3});
    assertSummary(summarizeInterval(worked, 0, 5), {-4, 2, 0, 6});
    hot100::p0053::Solution base;
    for (int length = 1, combinations = 3; length <= 8; ++length, combinations *= 3) {
        for (int code = 0; code < combinations; ++code) {
            std::vector<int> values(static_cast<std::size_t>(length));
            int rest = code;
            const int alphabet[]{-2, 0, 3};
            for (int& value : values) { value = alphabet[rest % 3]; rest /= 3; }
            const auto want = bruteSummary(values, 0, values.size());
            assertSummary(summarizeInterval(values, 0, values.size()), want);
            assert(maxSubArrayDivideAndConquer(values) == base.maxSubArray(values));
            for (std::size_t cut = 1; cut < values.size(); ++cut) {
                assertSummary(mergeIntervals(summarizeInterval(values, 0, cut),
                                             summarizeInterval(values, cut, values.size())), want);
            }
        }
    }
    const std::vector<int> positiveBound(100000, 10000), negativeBound(100000, -10000);
    assert(maxSubArrayDivideAndConquer(positiveBound) == 1000000000);
    assert(maxSubArrayDivideAndConquer(negativeBound) == -10000);
}

static double sortedMedian(std::vector<int> values) {
    std::sort(values.begin(), values.end());
    const std::size_t upper = values.size() / 2;
    const std::size_t lower = (values.size() - 1) / 2;
    return (static_cast<double>(values[lower]) + static_cast<double>(values[upper])) / 2.0;
}

static void checkPrefixes(const std::vector<int>& stream) {
    MedianBandCounts counts;
    MedianWithOrderedOutliers exact;
    hot100::p0295::MedianFinder base;
    std::vector<int> reference;
    for (int value : stream) {
        counts.addNum(value); exact.addNum(value); base.addNum(value);
        reference.push_back(value);
        const double want = sortedMedian(reference);
        assert(exact.findMedian() == want);
        assert(base.findMedian() == want);
        const auto ranks = medianRanks(reference.size());
        const bool inBand = counts.rankIsInBand(ranks.first) && counts.rankIsInBand(ranks.second);
        if (inBand) assert(counts.findMedian() == want);
        else {
            bool caught = false;
            try { (void)counts.findMedian(); }
            catch (const std::domain_error&) { caught = true; }
            assert(caught);
        }
    }
}

static void testBoundedMedians() {
    BoundedMedian101 empty;
    bool caught = false;
    try { (void)empty.findMedian(); }
    catch (const std::out_of_range&) { caught = true; }
    assert(caught);
    for (int invalid : {-1, 101}) {
        caught = false;
        try { empty.addNum(invalid); }
        catch (const std::invalid_argument&) { caught = true; }
        assert(caught);
    }
    BoundedMedian101 worked;
    for (int value : {0, 5, 5, 80, 100, 100}) worked.addNum(value);
    assert(worked.findMedian() == 42.5);
    BoundedMedian101 random;
    std::vector<int> reference;
    std::mt19937 rng(295101);
    for (int i = 0; i < 2500; ++i) {
        const int value = static_cast<int>(rng() % 101);
        random.addNum(value); reference.push_back(value);
        assert(random.findMedian() == sortedMedian(reference));
    }
    checkPrefixes({-8, -1, -1, 0, 100, 101, 900, 900});
    checkPrefixes({INT_MIN, INT_MAX, INT_MIN, 50, INT_MAX});
    for (int length = 1, combinations = 7; length <= 5; ++length, combinations *= 7) {
        for (int code = 0; code < combinations; ++code) {
            std::vector<int> stream(static_cast<std::size_t>(length));
            int rest = code;
            const int alphabet[]{-8, -1, 0, 1, 100, 101, 900};
            for (int& value : stream) { value = alphabet[rest % 7]; rest /= 7; }
            checkPrefixes(stream);
        }
    }
    // Exactly 99% in band; below count offsets the two middle ranks to 49,50.
    MedianBandCounts promised;
    promised.addNum(-9);
    for (int i = 0; i < 49; ++i) promised.addNum(10);
    for (int i = 0; i < 50; ++i) promised.addNum(90);
    assert(promised.belowCount() == 1 && promised.aboveCount() == 0);
    assert(promised.selectBandRank(50) == 10);
    assert(promised.selectBandRank(51) == 90);
    assert(promised.findMedian() == 50);
    // The final prefix has 99% in band. The first two prefixes require the exact outlier.
    std::vector<int> finalPromise{-100000};
    finalPromise.insert(finalPromise.end(), 99, 50);
    checkPrefixes(finalPromise);
    std::vector<int> upperPromise{100000};
    upperPromise.insert(upperPromise.end(), 99, 50);
    checkPrefixes(upperPromise);
    // Odd count, duplicates, both outlier sides, and 99% at an even final size.
    std::vector<int> bothSides{-100000, 100000};
    bothSides.insert(bothSides.end(), 99, 10);
    bothSides.insert(bothSides.end(), 99, 90);
    checkPrefixes(bothSides);
    BoundedMedian101 insertionBound;
    for (int i = 0; i < 50000; ++i) insertionBound.addNum(i % 2 == 0 ? 0 : 100);
    assert(insertionBound.findMedian() == 50);
}

static void testTeachingExamples() {
    hot100::p0072::Solution distance;
    assert(distance.minDistance("abc", "yabd") == 2);
    assert(distance.minDistance("yabd", "abc") == 2);
    hot100::p0076::Solution window;
    assert(window.minWindow("cabba", "aba") == "abba");
    assert(window.minWindow("cabbaa", "aba") == "baa");
    hot100::p0198::Solution robber;
    std::vector<int> prefix{4, 1};
    assert(robber.rob(prefix) == 4);
}

int main() {
    testDivideAndConquer();
    testBoundedMedians();
    testTeachingExamples();
    std::cout << "Follow-up variants: divide/conquer, exact band/outlier ranks, and teaching examples PASS\n";
}
