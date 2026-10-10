# 347. 前 K 个高频元素：C++17 推导与实现

> 分类：堆 · 难度：MEDIUM · 官方 Hot 100 顺序：75/100

## 题意与调用约定

返回出现次数最高的k个不同整数，结果顺序不限，k不超过不同值数量。官方保证答案唯一；测试额外允许频次并列，并按“选中频次不低于未选中”验证。

题面与最新约束见 [力扣中文官方题目](https://leetcode.cn/problems/top-k-frequent-elements/) / [LeetCode 官方英文题目](https://leetcode.com/problems/top-k-frequent-elements/)。下文给出独立编写的推导、示例与实现；测试是本地验证，没有声称在线评测已通过。

## 从直觉到优化

先计数再把不同值按频次排序要 O(n+u log u)。频次有一个强限制：只可能是1到n。因此可把“频次”直接作为桶下标，随后从高频桶向低频桶取值；无需比较排序，也无需堆操作。

## 不变量与正确性

frequency[value] 是完整输入中的次数；buckets[f] 恰好包含频次为f的所有不同值。从n往下扫描时，已经取出的项不可能比后续项频次更低；取满k即为合法前k高频集合。每个不同值只进入一个桶。

## 算法步骤

1. 用 unordered_map 统计所有值的出现次数。
2. 分配 n+1 个频次桶，把不同值放到对应频率下标。
3. 从n向1扫描桶，逐项输出。
4. 取满k时停止；并列顺序无关。

## 手工追踪

[4,4,4,9,9,2] 得到频次4→3、9→2、2→1。bucket[3]=[4]，bucket[2]=[9]，bucket[1]=[2]。k=2 从3桶取4，从2桶取9即可，不再访问1桶。

## 完整 C++17 实现

以下内容来自本课 [可编译头文件](../../../algorithms/cpp/solutions/p0347_top_k_frequent_elements.hpp)，namespace用于让100题可同时进入同一工程。在线提交时一般只保留评测要求的类声明，并按平台要求处理标准库命名；本地辅助头文件负责统一导入标准库。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0347 {
class Solution {
public:
    std::vector<int> topKFrequent(std::vector<int>& nums, int k) {
        std::unordered_map<int,int> frequency;
        for (int value : nums) ++frequency[value];
        std::vector<std::vector<int>> buckets(nums.size() + 1);
        for (const auto& entry : frequency) buckets[entry.second].push_back(entry.first);
        std::vector<int> answer;
        for (int f = static_cast<int>(nums.size()); f > 0 && static_cast<int>(answer.size()) < k; --f) {
            for (int value : buckets[f]) {
                answer.push_back(value);
                if (static_cast<int>(answer.size()) == k) break;
            }
        }
        return answer;
    }
};
} // namespace hot100::p0347
```

## 复杂度核算

设n为元素数、u为不同值数。哈希平均时间 O(n)，桶创建与扫描 O(n+u)，总平均 O(n)；哈希恶意碰撞最坏可达 O(n²)。辅助空间 O(n+u)，其中 n+1 个 vector 桶本身也占 O(n)，输出 O(k)。

## C++ 常见陷阱

- unordered_map 的迭代顺序不稳定，不能把答案顺序作为正确性要求。
- 不能只按值排序后取k项，目标排序键是出现频次。
- 遍历 map 的 const 引用避免复制键值；分桶时不再修改 map，避免迭代器因 rehash 失效。

## 测试与运行

[完整测试源文件](../../../algorithms/cpp/tests/test_p0347.cpp)：1200组固定种子数组对每个合法k检查长度、去重、成员存在与选中频率阈值；不假定unordered_map输出顺序或并列答案固定。

从仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0347.cpp -o /tmp/hot100-cpp/test_p0347
/tmp/hot100-cpp/test_p0347
# 环境支持时增加运行期检查：
g++ -std=c++17 -Wall -Wextra -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0347.cpp -o /tmp/hot100-cpp/test_p0347_san
ASAN_OPTIONS=detect_leaks=0 /tmp/hot100-cpp/test_p0347_san
```

本次环境的LeakSanitizer受ptrace限制，因此命令显式关闭泄漏检测；AddressSanitizer与UndefinedBehaviorSanitizer仍启用。支持泄漏检测的本地环境可移除该环境变量。断言通过说明覆盖输入上的本地结果正确，不能替代证明、完整平台约束核对或在线评测。

## 面试追问

- u很小但n很大时，大小k小顶堆是否更节省桶数组空间？
- 若需要稳定的并列规则，例如值小者优先，应在哪一步增加排序？

## 练习与提示

- 练习1：实现 O(u log k) 的堆选择阶段。提示：堆顶放已保留项中频次最低者。
- 练习2：设计正确性检查器处理并列。提示：检验长度、去重、成员存在与频率阈值。
