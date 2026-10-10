# 322. 零钱兑换：C++17 推导与实现

> 分类：动态规划 · 难度：MEDIUM · 官方 Hot 100 顺序：85/100

## 题意与调用约定

硬币面额为正整数，每种可无限使用，求组成非负 amount 的最少硬币数；无法组成返回 -1。零金额不用硬币，返回0。

题面与最新约束见 [力扣中文官方题目](https://leetcode.cn/problems/coin-change/) / [LeetCode 官方英文题目](https://leetcode.com/problems/coin-change/)。下文给出独立编写的推导、示例与实现；测试是本地验证，没有声称在线评测已通过。

## 从直觉到优化

总优先拿最大硬币不总正确，例如[1,4,6]凑8时6+1+1比4+4更多。固定最后一枚硬币后，问题变成较小金额的最优解。枚举每种最后硬币即可DP；无限使用体现在较小总额已允许用过当前面额。

## 不变量与正确性

处理 total 前，所有更小金额的 best 都是最少枚数或不可达标记。任意合法方案都有某枚最后硬币c，其剩余金额 total-c 已被计算。取 min(best[total-c]+1) 不漏方案，也不会形成非硬币和；不可达来源不会击败可行上界。

## 算法步骤

1. best[0]=0，其余设 amount+1，因为正面额方案最多用 amount 枚。
2. 按总金额1到amount递增处理。
3. 枚举不大于当前金额的正硬币，从 total-coin 转移加1。
4. 最终标记仍超过amount则返回 -1。

## 手工追踪

coins=[1,4,6]、amount=8。best[4]=1，best[6]=1；求8时来源7+1为3枚、4+4为2枚、2+6为3枚，所以 best[8]=2。面额4与6凑7时所有路径都不可达。

## 完整 C++17 实现

以下内容来自本课 [可编译头文件](../../../algorithms/cpp/solutions/p0322_coin_change.hpp)，namespace用于让100题可同时进入同一工程。在线提交时一般只保留评测要求的类声明，并按平台要求处理标准库命名；本地辅助头文件负责统一导入标准库。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0322 {
class Solution {
public:
    int coinChange(std::vector<int>& coins, int amount) {
        std::vector<int> best(amount + 1, amount + 1);
        best[0] = 0;
        for (int total = 1; total <= amount; ++total) {
            for (int coin : coins) {
                if (coin > 0 && coin <= total)
                    best[total] = std::min(best[total], best[total-coin] + 1);
            }
        }
        return best[amount] <= amount ? best[amount] : -1;
    }
};
} // namespace hot100::p0322
```

## 复杂度核算

设C为面额数量、A为金额，时间 O(AC)，额外空间 O(A)。输入中重复面额只增加重复工作，不改变答案；无递归栈。

## C++ 常见陷阱

- 不能用 INT_MAX 后直接 +1，否则溢出；amount+1 是约束内安全上界。
- amount 非负且 amount+1 可表示是前提，超大金额不适合此伪多项式算法。
- 面额0或负数在官方输入外；代码忽略它们以防自循环，但不声称完整验证非法输入。

## 测试与运行

[完整测试源文件](../../../algorithms/cpp/tests/test_p0322.cpp)：1200组固定种子面额与金额，与独立BFS最短路对拍；包含贪心失败例、不可达、零金额与空面额表。

从仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0322.cpp -o /tmp/hot100-cpp/test_p0322
/tmp/hot100-cpp/test_p0322
# 环境支持时增加运行期检查：
g++ -std=c++17 -Wall -Wextra -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0322.cpp -o /tmp/hot100-cpp/test_p0322_san
ASAN_OPTIONS=detect_leaks=0 /tmp/hot100-cpp/test_p0322_san
```

本次环境的LeakSanitizer受ptrace限制，因此命令显式关闭泄漏检测；AddressSanitizer与UndefinedBehaviorSanitizer仍启用。支持泄漏检测的本地环境可移除该环境变量。断言通过说明覆盖输入上的本地结果正确，不能替代证明、完整平台约束核对或在线评测。

## 面试追问

- 若改为每种硬币只有一枚，为什么更新顺序必须变化？
- 最少硬币数与组合数量不同，状态与循环次序分别有什么差别？

## 练习与提示

- 练习1：输出实际硬币组合。提示：保存每个金额的最后选择，直到回到0。
- 练习2：限制每种面额最多用t次。提示：二进制拆分或引入数量维度。
