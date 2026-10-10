# 70. 爬楼梯：C++17 推导与实现

> 分类：动态规划 · 难度：EASY · 官方 Hot 100 顺序：81/100

## 题意与调用约定

每次上1级或2级，计算恰好到达第n级的不同步序数量。官方 n 为1到45；本课把 n=0 定义为一种空步序。

题面与最新约束见 [力扣中文官方题目](https://leetcode.cn/problems/climbing-stairs/) / [LeetCode 官方英文题目](https://leetcode.com/problems/climbing-stairs/)。下文给出独立编写的推导、示例与实现；测试是本地验证，没有声称在线评测已通过。

## 从直觉到优化

递归枚举最后走1级或2级会反复求相同子问题。到第n级的路径按最后一步分成两个互斥集合，来源分别是n-1与n-2。记忆化或数组DP消掉重复后，只需保留前两个值就能滚动计算。

## 不变量与正确性

第 step 次循环前，before=f(step-2)，current=f(step-1)；next 是两种最后一步来源之和。移动变量后得到 f(step-1)、f(step)，不变量延续。基础 f(0)=1、f(1)=1 代表空走法和唯一一步走法。

## 算法步骤

1. 初始化 f(0)、f(1) 为1。
2. 从2到n依次计算 next=before+current。
3. 先保存 next，再移动前两项，避免覆盖依赖。
4. 循环结束返回 current。

## 手工追踪

n=6：状态对从(1,1)出发，依次成为(1,2)、(2,3)、(3,5)、(5,8)、(8,13)。第6级的13种路径由第5级8种加上第4级5种组成。

## 完整 C++17 实现

以下内容来自本课 [可编译头文件](../../../algorithms/cpp/solutions/p0070_climbing_stairs.hpp)，namespace用于让100题可同时进入同一工程。在线提交时一般只保留评测要求的类声明，并按平台要求处理标准库命名；本地辅助头文件负责统一导入标准库。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0070 {
class Solution {
public:
    int climbStairs(int n) {
        if (n < 0) throw std::invalid_argument("negative stairs");
        long long before = 1, current = 1;
        for (int step = 2; step <= n; ++step) {
            long long next = before + current;
            before = current; current = next;
        }
        return static_cast<int>(current);
    }
};
} // namespace hot100::p0070
```

## 复杂度核算

时间 O(n)，额外空间 O(1)。朴素递归时间指数级、调用栈 O(n)；本实现没有递归栈。

## C++ 常见陷阱

- 官方n<=45时答案可由本目标的32位有符号 int 返回。n=45为1836311903，n=46为2971215073，后者超过32位有符号 int 上界2147483647；这不是所有C++17 int 的共同上界。本地目标由 [support.hpp](../../../algorithms/cpp/support.hpp) 断言，详见[表示契约](../SOURCES.md)。当前接口只承诺官方范围及n=0扩展，扩大n时需同时检查中间值与返回类型；long long 也不是无限精度。
- 不能先 before=current 再 current=before+current，否则读不到旧 before。
- n=0 的结果是数学扩展，面试时要先说清楚而不是擅自当作官方要求。

## 测试与运行

[完整测试源文件](../../../algorithms/cpp/tests/test_p0070.cpp)：对0到23的每个n与朴素递归枚举对拍，并检查n=45的边界答案；覆盖n=0约定和基础项。

从仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0070.cpp -o /tmp/hot100-cpp/test_p0070
/tmp/hot100-cpp/test_p0070
# 环境支持时增加运行期检查：
g++ -std=c++17 -Wall -Wextra -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0070.cpp -o /tmp/hot100-cpp/test_p0070_san
ASAN_OPTIONS=detect_leaks=0 /tmp/hot100-cpp/test_p0070_san
```

本次环境的LeakSanitizer受ptrace限制，因此命令显式关闭泄漏检测；AddressSanitizer与UndefinedBehaviorSanitizer仍启用。支持泄漏检测的本地环境可移除该环境变量。断言通过说明覆盖输入上的本地结果正确，不能替代证明、完整平台约束核对或在线评测。

## 面试追问

- 台阶步长改成集合{1,3,4}时递推式是什么？
- 可用矩阵快速幂得到 O(log n) 吗？何时整数位数成本不能忽略？

## 练习与提示

- 练习1：允许步长1、2、3。提示：f(n)=f(n-1)+f(n-2)+f(n-3)，越界来源计0。
- 练习2：统计恰好使用k次两级步的方案。提示：总步数是n-k，再选择k个位置。
