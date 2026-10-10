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

已有t项时 addNum 为 O(log(t+1))，findMedian 为 O(1)。两个堆合计存全部t项，空间 O(t)；priority_queue 底层 vector 的容量增长为摊还成本。

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

- 所有数字位于0到100之间时，频数数组如何改进？
- 加入删除过期元素后怎样支持滑动窗口中位数？讨论延迟删除与逻辑堆大小。

## 练习与提示

- 练习1：支持查询第25百分位。提示：调整目标两部分的数量比例，并明确百分位定义。
- 练习2：每次输入后与排序基线对拍。提示：覆盖重复值、递增流、递减流和 INT_MAX。
