# 300. 最长递增子序列：C++17 推导与实现

> 分类：动态规划 · 难度：MEDIUM · 官方 Hot 100 顺序：87/100

## 题意与调用约定

求最长严格递增子序列长度。子序列可跳过元素但保持相对顺序，不必连续；相等值不能延长严格递增。

题面与最新约束见 [力扣中文官方题目](https://leetcode.cn/problems/longest-increasing-subsequence/) / [LeetCode 官方英文题目](https://leetcode.com/problems/longest-increasing-subsequence/)。下文给出独立编写的推导、示例与实现；测试是本地验证，没有声称在线评测已通过。

## 从直觉到优化

经典dp[i]枚举所有较早且更小的值需要 O(n²)。另一种状态按“长度”而非“终点”索引：tails[len-1] 记录已处理前缀中长度len递增子序列的最小末值。末值越小，未来越容易延长，所以只需保留最有利代表。tails 本身递增，能二分替换位置。

## 不变量与正确性

tails[k] 是长度k+1的可行递增子序列最小末值。遇value时，第一个不小于它的位置p之前都更小，可以把长度p的代表延长成长度p+1；用value替换p不会减少已有最大长度，只会改善末值。若无该位置，value比所有末值更大，最大长度加一。

## 算法步骤

1. 维护初始为空的 tails。
2. 对每个value二分找到第一个 >=value 的位置。
3. 尾后位置就追加；否则覆盖该位置。
4. 只返回 tails 长度，不把 tails 当成原数组的一条实际子序列。

## 手工追踪

[4,10,5,6,2,8] 的 tails 依次[4]、[4,10]、[4,5]、[4,5,6]、[2,5,6]、[2,5,6,8]，长度4。最终tails中的2在原数组出现较晚，所以整个[2,5,6,8]不一定是合法原序列；可行答案如[4,5,6,8]。

## 完整 C++17 实现

以下内容来自本课 [可编译头文件](../../../algorithms/cpp/solutions/p0300_longest_increasing_subsequence.hpp)，namespace用于让100题可同时进入同一工程。在线提交时一般只保留评测要求的类声明，并按平台要求处理标准库命名；本地辅助头文件负责统一导入标准库。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0300 {
class Solution {
public:
    int lengthOfLIS(std::vector<int>& nums) {
        std::vector<int> tails;
        for (int value : nums) {
            auto it = std::lower_bound(tails.begin(), tails.end(), value);
            if (it == tails.end()) tails.push_back(value);
            else *it = value;
        }
        return static_cast<int>(tails.size());
    }
};
} // namespace hot100::p0300
```

## 复杂度核算

时间 O(n log(n+1))，更精确为 O(n log(L+1))，L是答案长度；辅助空间 O(L)。无递归，返回标量。

## C++ 常见陷阱

- strictly increasing 用 lower_bound；换 upper_bound 就变为非递减子序列。
- push_back 可能使 it 失效，本实现追加后不再使用 it。
- 空输入无需读取 nums[0]，自然返回0。tails 的元素不能直接作为路径恢复答案。

## 测试与运行

[完整测试源文件](../../../algorithms/cpp/tests/test_p0300.cpp)：1100组固定种子短数组枚举全部子序列掩码，检查严格递增后取最大长度；含重复值和空数组。

从仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0300.cpp -o /tmp/hot100-cpp/test_p0300
/tmp/hot100-cpp/test_p0300
# 环境支持时增加运行期检查：
g++ -std=c++17 -Wall -Wextra -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0300.cpp -o /tmp/hot100-cpp/test_p0300_san
ASAN_OPTIONS=detect_leaks=0 /tmp/hot100-cpp/test_p0300_san
```

本次环境的LeakSanitizer受ptrace限制，因此命令显式关闭泄漏检测；AddressSanitizer与UndefinedBehaviorSanitizer仍启用。支持泄漏检测的本地环境可移除该环境变量。断言通过说明覆盖输入上的本地结果正确，不能替代证明、完整平台约束核对或在线评测。

## 面试追问

- 如何用前驱下标恢复一条真正的最长递增子序列？
- 若要统计最长递增子序列的条数，为什么单个最小末值不够？

## 练习与提示

- 练习1：改为最长非递减子序列。提示：upper_bound 查第一个 >value 的位置。
- 练习2：恢复长度及路径。提示：tails 保存代表下标，并为每项保存上一长度代表的下标。
