# 279. 完全平方数：C++17 推导与实现

> 分类：动态规划 · 难度：MEDIUM · 官方 Hot 100 顺序：84/100

## 题意与调用约定

把非负整数n表示成若干正完全平方数之和，返回最少项数。每个平方数可重复使用；n=0作为扩展返回0，官方n为正。

题面与最新约束见 [力扣中文官方题目](https://leetcode.cn/problems/perfect-squares/) / [LeetCode 官方英文题目](https://leetcode.com/problems/perfect-squares/)。下文给出独立编写的推导、示例与实现；测试是本地验证，没有声称在线评测已通过。

## 从直觉到优化

试遍平方数组合会产生重复子问题。固定总和total，最后加入的平方数可能是任意 r²<=total；此前的最优项数为 best[total-r²]。把各个最后选择的结果取最小，得到完整递推。平方数1保证每个正总和都可达。

## 不变量与正确性

按总和从小到大处理时，所有 best[total-r²] 都已是最优。任意表示法的最后一项都属于枚举平方数，删去该项得到较小子问题；反过来每个子问题方案加一个平方都合法。因此递推上下界一致，得到最优值。

## 算法步骤

1. 分配0到n的最少项数组，best[0]=0，其余设不可达上界 n+1。
2. 按total从1到n扫描。
3. 枚举所有 r²<=total，用子问题加1更新。
4. 返回 best[n]。

## 手工追踪

n=13：平方候选1、4、9。best[12]+1=4，best[9]+1=2，best[4]+1=2，所以 best[13]=2，对应9+4。n=23无法用三个以内平方完成，DP得到4。

## 完整 C++17 实现

以下内容来自本课 [可编译头文件](../../../algorithms/cpp/solutions/p0279_perfect_squares.hpp)，namespace用于让100题可同时进入同一工程。在线提交时一般只保留评测要求的类声明，并按平台要求处理标准库命名；本地辅助头文件负责统一导入标准库。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0279 {
class Solution {
public:
    int numSquares(int n) {
        std::vector<int> best(n + 1, n + 1);
        best[0] = 0;
        for (int total = 1; total <= n; ++total)
            for (int root = 1; root <= total / root; ++root)
                best[total] = std::min(best[total], best[total-root*root] + 1);
        return best[n];
    }
};
} // namespace hot100::p0279
```

## 复杂度核算

时间 Θ(n√n)，平方候选次数之和与 n√n 同阶；额外空间 O(n)，返回标量，无递归。

## C++ 常见陷阱

- root*root 可能溢出，因此循环条件写 root<=total/root，乘法只在已确认结果<=total时执行。
- n<0不在接口前提，不能拿它初始化 vector；生产接口需显式验证。
- 平方数可重复，这是完全背包；不能误用0/1背包的倒序容量更新。

## 测试与运行

[完整测试源文件](../../../algorithms/cpp/tests/test_p0279.cpp)：对0到400的每个n与从0加平方数的BFS最短路对拍；另验证完全平方10000只需1项。

从仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0279.cpp -o /tmp/hot100-cpp/test_p0279
/tmp/hot100-cpp/test_p0279
# 环境支持时增加运行期检查：
g++ -std=c++17 -Wall -Wextra -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0279.cpp -o /tmp/hot100-cpp/test_p0279_san
ASAN_OPTIONS=detect_leaks=0 /tmp/hot100-cpp/test_p0279_san
```

本次环境的LeakSanitizer受ptrace限制，因此命令显式关闭泄漏检测；AddressSanitizer与UndefinedBehaviorSanitizer仍启用。支持泄漏检测的本地环境可移除该环境变量。断言通过说明覆盖输入上的本地结果正确，不能替代证明、完整平台约束核对或在线评测。

## 面试追问

- 可把0..n视为图，用 BFS 找最少平方数；节点与边是什么？
- 四平方定理提供何种更快数论方案？为何仍要解释定理适用前提？

## 练习与提示

- 练习1：返回一组最优平方分解。提示：记录使 best 改善的最后平方数。
- 练习2：实现BFS对拍。提示：从0加平方到新总和，每条边成本1。
