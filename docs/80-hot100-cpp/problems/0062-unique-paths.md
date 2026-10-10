# 62. 不同路径：C++17 推导与实现

> 分类：多维动态规划 · 难度：MEDIUM · 官方 Hot 100 顺序：91/100

## 题意与调用约定

m行n列的规则网格，从左上走到右下，每步只能向右或向下，求路径数量。官方维度为正、答案保证在32位有符号范围；零维度扩展返回0。

题面与最新约束见 [力扣中文官方题目](https://leetcode.cn/problems/unique-paths/) / [LeetCode 官方英文题目](https://leetcode.com/problems/unique-paths/)。下文给出独立编写的推导、示例与实现；测试是本地验证，没有声称在线评测已通过。

## 从直觉到优化

直接枚举路线按每一步分叉，路径数量可能指数增长。每个格子的路径都来自上方或左方，两集合按最后一步互斥；二维DP是来源之和。当前行只依赖上一行与本行左项，可以复用一行。由于没有障碍，交换行列不影响答案，可让滚动数组更短。

## 不变量与正确性

内层处理 col 前，ways[col] 仍代表上一行同列路径数，ways[col-1] 已代表本行左列。相加后成为本格路径数。第一行与第一列只有一条直走路径，ways 初始全1且col从1开始保证边界。

## 算法步骤

1. 非正维度返回0，并将较短维度作为列数。
2. 第一行路径数全设1。
3. 逐行从左向右更新 ways[col]+=ways[col-1]。
4. 最后一列的滚动状态即右下角答案。

## 手工追踪

3×4：先交换后可以使用长度3的数组。第一行[1,1,1]，第二行[1,2,3]，第三行[1,3,6]，第四行[1,4,10]。与原3×4相同，共10条。每次更新都同时读“旧上方”和“新左方”。

## 完整 C++17 实现

以下内容来自本课 [可编译头文件](../../../algorithms/cpp/solutions/p0062_unique_paths.hpp)，namespace用于让100题可同时进入同一工程。在线提交时一般只保留评测要求的类声明，并按平台要求处理标准库命名；本地辅助头文件负责统一导入标准库。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0062 {
class Solution {
public:
    int uniquePaths(int m, int n) {
        if (m <= 0 || n <= 0) return 0;
        if (n > m) std::swap(m, n);
        std::vector<long long> ways(n, 1);
        for (int row = 1; row < m; ++row)
            for (int col = 1; col < n; ++col)
                ways[col] += ways[col-1];
        return static_cast<int>(ways.back());
    }
};
} // namespace hot100::p0062
```

## 复杂度核算

时间 O(mn)，辅助空间 O(min(m,n))。返回标量；无递归。内部用 long long，最终返回 int 依赖官方答案上限。

## C++ 常见陷阱

- 滚动更新必须从左到右，反向会读取上一行左值而非本行左值。
- 路径数无障碍时单调增加，官方终点答案范围也约束中间值；任意大维度仍会超过 long long。
- 组合数公式的分子先乘后除可能溢出，即使最终答案可表示也要分析中间值。

## 测试与运行

[完整测试源文件](../../../algorithms/cpp/tests/test_p0062.cpp)：对1到8行/列所有网格与递归枚举对拍，并验证转置对称；另检查零维度、单行与17×17的大答案。

从仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0062.cpp -o /tmp/hot100-cpp/test_p0062
/tmp/hot100-cpp/test_p0062
# 环境支持时增加运行期检查：
g++ -std=c++17 -Wall -Wextra -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0062.cpp -o /tmp/hot100-cpp/test_p0062_san
ASAN_OPTIONS=detect_leaks=0 /tmp/hot100-cpp/test_p0062_san
```

本次环境的LeakSanitizer受ptrace限制，因此命令显式关闭泄漏检测；AddressSanitizer与UndefinedBehaviorSanitizer仍启用。支持泄漏检测的本地环境可移除该环境变量。断言通过说明覆盖输入上的本地结果正确，不能替代证明、完整平台约束核对或在线评测。

## 面试追问

- 加入障碍物后初始化与更新规则如何改变？
- 组合数 C(m+n-2,m-1) 与DP递推怎样互相验证？

## 练习与提示

- 练习1：实现障碍版。提示：障碍处 ways[col]=0，首格也要检查。
- 练习2：用小网格递归枚举验证滚动方向。提示：边界行列作为1，比较正反两种循环。
