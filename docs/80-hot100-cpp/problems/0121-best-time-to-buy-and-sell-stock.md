# 121. 买卖股票的最佳时机：C++17 推导与实现

> 分类：贪心算法 · 难度：EASY · 官方 Hot 100 顺序：77/100

## 题意与调用约定

最多做一次买入和一次以后卖出，求最大非负收益；可以不交易，返回0。价格为非负整数；空输入是实现的扩展边界。

题面与最新约束见 [力扣中文官方题目](https://leetcode.cn/problems/best-time-to-buy-and-sell-stock/) / [LeetCode 官方英文题目](https://leetcode.com/problems/best-time-to-buy-and-sell-stock/)。下文给出独立编写的推导、示例与实现；测试是本地验证，没有声称在线评测已通过。

## 从直觉到优化

枚举买卖日需要 O(n²)。固定卖出日时，最佳买入价一定是它之前的最低价；未来价格不会改善已经结束的交易。扫描只需要保存历史最低价，以及已见过的最佳收益。

## 不变量与正确性

处理第i天后，lowest 是第0到i天最低价，best 是这些日子中所有合法交易的最大非负收益。把当天也放入最低价只增加“当天买当天卖”的0收益候选，不会产生正收益的非法交易。因此更新顺序可以先降最低价再算收益。

## 算法步骤

1. 最低价初始化为 long long 最大值，最佳收益初始化0。
2. 逐日更新最低价。
3. 用当天价减最低价更新最佳收益。
4. 返回最大收益；没有上涨自然保持0。

## 手工追踪

[9,6,8,3,10,4]：最低价依次9、6、6、3、3、3；当天候选收益0、0、2、0、7、1。最佳交易在价格3后以10卖出，收益7。初期8-6的2不会阻止后续更好选择。

## 完整 C++17 实现

以下内容来自本课 [可编译头文件](../../../algorithms/cpp/solutions/p0121_best_time_to_buy_and_sell_stock.hpp)，namespace用于让100题可同时进入同一工程。在线提交时一般只保留评测要求的类声明，并按平台要求处理标准库命名；本地辅助头文件负责统一导入标准库。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0121 {
class Solution {
public:
    int maxProfit(std::vector<int>& prices) {
        long long lowest = std::numeric_limits<long long>::max(), best = 0;
        for (int price : prices) {
            lowest = std::min(lowest, static_cast<long long>(price));
            best = std::max(best, static_cast<long long>(price) - lowest);
        }
        return static_cast<int>(best);
    }
};
} // namespace hot100::p0121
```

## 复杂度核算

时间 O(n)，额外空间 O(1)，返回标量；无递归、无哈希。

## C++ 常见陷阱

- 不能用全局最大价减全局最小价，因为最大价可能出现在最低价之前。
- 差值用 long long，返回 int 依赖官方价格范围；泛化到任意 int 价格应改返回类型。
- best 从0开始代表不交易，不应在单元素输入返回负收益。

## 测试与运行

[完整测试源文件](../../../algorithms/cpp/tests/test_p0121.cpp)：2000组固定种子价格数组与枚举所有合法买卖日对拍；含递减、单项与空数组。

从仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0121.cpp -o /tmp/hot100-cpp/test_p0121
/tmp/hot100-cpp/test_p0121
# 环境支持时增加运行期检查：
g++ -std=c++17 -Wall -Wextra -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0121.cpp -o /tmp/hot100-cpp/test_p0121_san
ASAN_OPTIONS=detect_leaks=0 /tmp/hot100-cpp/test_p0121_san
```

本次环境的LeakSanitizer受ptrace限制，因此命令显式关闭泄漏检测；AddressSanitizer与UndefinedBehaviorSanitizer仍启用。支持泄漏检测的本地环境可移除该环境变量。断言通过说明覆盖输入上的本地结果正确，不能替代证明、完整平台约束核对或在线评测。

## 面试追问

- 允许多次交易后，为什么可以累计所有相邻正差？
- 加冷冻期、手续费或最多k次交易时，需要哪些额外状态？

## 练习与提示

- 练习1：返回买入和卖出下标。提示：更新最低价时记索引，只在收益严格变大时记录交易。
- 练习2：构造全局最大减最小不合法的反例。提示：让最高价在第一天。
