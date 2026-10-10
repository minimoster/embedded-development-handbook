# 295. 数据流的中位数：C++17 推导与实现

> 分类：堆 · 难度：HARD · 官方 Hot 100 顺序：76/100

## 题意与调用约定

持续接收整数，并在任意非空时刻查询所有已接收数据的中位数。不要求保留插入顺序。官方只在已有数据时查询；教学实现为空时抛出异常。

题面与最新约束见 [力扣中文官方题目](https://leetcode.cn/problems/find-median-from-data-stream/) / [LeetCode 官方英文题目](https://leetcode.com/problems/find-median-from-data-stream/)。下文给出独立编写的推导、示例与实现；测试是本地验证，没有声称在线评测已通过。

## 从直觉到优化

每次插入后排序使插入很贵。中位数只需要较小一半的最大值和较大一半的最小值。两个堆分别维护这两部分：lower 为大顶堆，upper 为小顶堆。让 lower 项数等于 upper 或比它多1，就能用堆顶直接回答。

## 不变量与正确性

所有 lower 元素<=所有 upper 元素，且 |lower|=|upper| 或 |lower|=|upper|+1。先插入lower，再把其最大项移给upper，恢复顺序分界；若upper多一项，再将其最小项移回lower，恢复数量。移动的都是边界项，所以两个性质同时保持。

## 算法步骤

1. 新值先进入 lower。
2. 把 lower 最大值移到 upper，保证两堆的排序分界。
3. 若 upper 更大，把 upper 最小值移回 lower。
4. 奇数项取 lower.top；偶数项取两堆顶平均。

## 手工追踪

依次输入8、2、11、-1。第一步 lower=[8]；第二步分成 lower=[2]、upper=[8]，中位数5；第三步 lower含[2,8]、upper含[11]，中位数8；第四步 lower含[-1,2]、upper含[8,11]，中位数5。

## 完整 C++17 实现

以下内容来自本课 [可编译头文件](../../../algorithms/cpp/solutions/p0295_find_median_from_data_stream.hpp)，namespace用于让100题可同时进入同一工程。在线提交时一般只保留评测要求的类声明，并按平台要求处理标准库命名；本地辅助头文件负责统一导入标准库。

```cpp
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
```

## 复杂度核算

已有t项时，addNum **均摊** O(log(t+1))；单次调用若底层vector扩容，最坏可为O(t)。堆调整本身是O(log(t+1))，不能把它直接当作包含动态存储增长的单次最坏界。findMedian最坏O(1)，两个堆合计存全部t项，空间O(t)。依据见[N4659 priority_queue成员](https://timsong-cpp.github.io/cppwp/n4659/priqueue.members)和[vector末端插入的均摊界](https://timsong-cpp.github.io/cppwp/n4659/vector.overview)。

如果要求严格单次延迟界，必须另外约束容量、分配与元素移动，或选用满足相应操作界的数据结构，再证明整体界；以上大O不是嵌入式硬实时保证。

## C++ 常见陷阱

- 偶数平均必须先转 double 再加，避免 int 极值溢出。
- priority_queue 默认大顶堆；upper 明确使用 greater<int>。
- 不能以“两个堆个数接近”代替排序分界，错误分组仍会给错误中位数。

## 测试与运行

[完整测试源文件](../../../algorithms/cpp/tests/test_p0295.cpp)：3000次固定种子数据流，每次与排序后的中位数核对；专测奇偶数量、重复INT_MAX避免求和溢出、空查询异常。

从仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0295.cpp -o /tmp/hot100-cpp/test_p0295
/tmp/hot100-cpp/test_p0295
# 环境支持时增加运行期检查：
g++ -std=c++17 -Wall -Wextra -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0295.cpp -o /tmp/hot100-cpp/test_p0295_san
ASAN_OPTIONS=detect_leaks=0 /tmp/hot100-cpp/test_p0295_san
```

本次环境的LeakSanitizer受ptrace限制，因此命令显式关闭泄漏检测；AddressSanitizer与UndefinedBehaviorSanitizer仍启用。支持泄漏检测的本地环境可移除该环境变量。断言通过说明覆盖输入上的本地结果正确，不能替代证明、完整平台约束核对或在线评测。

## 面试追问

[官方题页的两类范围追问](https://leetcode.com/problems/find-median-from-data-stream/)需要分别讨论，且先明确99%承诺在哪些查询时刻成立。

### 全部值在[0,100]

用101项频数数组freq[v]和总数n，插入时只增加一项。以1为起始秩，两个中位数秩为k1=⌊(n-1)/2⌋+1、k2=⌊n/2⌋+1；奇数时两者相同，偶数时相邻。按值从0到100累计频数，第一个累计值达到k的桶就是第k小项，两个选中值先转double再求平均。插入最坏O(1)，查询最坏O(101)，存储O(101)；范围固定后后两者也可写O(1)，但保留101能说明常数来自哪里。

例如[0,5,5,80,100,100]有n=6、秩3和4；到值5时累计3项，到80时累计4项，所以中位数为(5+80)/2=42.5。重复值要按频数，而不是按不同桶计秩。

### 99%承诺对每次被查询的数据集成立

若每次查询时至少99%的已接收数都在[0,100]，设b为小于0的数量、a为大于100的数量。它们各自不超过0.01n，因而都小于两个中央秩到对应端点的距离；两个中位数秩必在带内。只需101项频数和b、a计数，不必保存离群值本身。

对带内的全局秩k，扫描时目标秩必须改为k-b；不能直接在频数数组中查第k项。精确条件为b<k<=n-a，两个中央秩都满足时才可仅靠频数回答。插入O(1)、查询O(101)、存储O(101)。这份计数法不通过裁剪数值或删除离群样本来改变总体秩。

例如n=100，数据为1个-9、49个10、50个90，恰好99%在带内。全局中央秩是50和51，减去b=1后是带内第49、50项，分别为10、90，中位数50。若忘记减b，两项会都落入90，错误地返回90。

### 99%只描述最终流，早期前缀也可查询

最终比例不能保证每个前缀。例如先来-100000，随后99个50：最终恰好99%在带内，但第一项的中位数是-100000，第二项时是(-100000+50)/2=-49975。只保存离群计数无法恢复这些数值；把它裁到0、丢弃它或始终返回带内桶都会给错。

这时保留带内频数，以及带外两侧的**精确有序值**。对每个目标秩k：

1. k<=b：查询下侧离群结构的第k小项。
2. b<k<=n-a：在带内查第k-b项。
3. k>n-a：查询上侧离群结构的第k-(n-a)小项。

[可编译练习](../../../algorithms/cpp/exercises/followup_variants.hpp)用std::multiset分别保存两侧离群项，保留重复值。设o是当前离群项数，带内插入O(1)，带外插入O(log(o+1))；中央秩都在带内时查询O(101)，若秩落在带外，标准multiset的迭代器推进最坏O(o)，所以一般查询界O(101+o)，存储O(101+o)。最终99%承诺只能给最终o<=0.01n，不能给任意早期前缀同样的比例。若要求带外第k项也在O(log o)查询，需要带子树大小的顺序统计结构；标准multiset并不自动提供这个操作。

练习同时提供全范围版、只存计数且拒绝带外中央秩的版本、任意前缀精确版。[独立回归](../../../algorithms/cpp/exercises/test_followup_variants.cpp)覆盖奇偶秩、重复桶、两侧离群、99%样例及最终承诺不覆盖早期前缀的反例，并与排序基线逐前缀核对。这些仍是LC295追问，不新增题单；总规模沿用原题最多50000次插入的范围。

其他变化：加入删除过期元素后怎样支持滑动窗口中位数？讨论延迟删除与逻辑堆大小。

## 练习与提示

- 练习1：支持查询第25百分位。提示：调整目标两部分的数量比例，并明确百分位定义。
- 练习2：每次输入后与排序基线对拍。提示：覆盖重复值、递增流、递减流和 INT_MAX。
