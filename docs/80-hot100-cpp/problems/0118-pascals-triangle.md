# 118. 杨辉三角：C++17 推导与实现

> 分类：动态规划 · 难度：EASY · 官方 Hot 100 顺序：82/100

## 题意与调用约定

生成前 numRows 行杨辉三角；每行两端为1，内部值等于上一行相邻两项之和。官方行数较小，int 能表示所有输出；实现额外支持0行。

题面与最新约束见 [力扣中文官方题目](https://leetcode.cn/problems/pascals-triangle/) / [LeetCode 官方英文题目](https://leetcode.com/problems/pascals-triangle/)。下文给出独立编写的推导、示例与实现；测试是本地验证，没有声称在线评测已通过。

## 从直觉到优化

单独按组合数公式计算每项也能做，但反复求阶乘既慢又容易溢出。相邻行之间存在局部依赖：当前行内部第c项只依赖上一行c-1与c。输出必须包含所有行，因此直接从已生成的上一行构造即可。

## 不变量与正确性

循环开始时 triangle 已正确包含第0到row-1行。新行全部初始化为1，保证两个边界；内部两项相加对应“从上一行左上或右上到此处”的两种来源，得到正确组合数。新行完成后才加入 triangle，不会混读正在写入的数据。

## 算法步骤

1. 从第0行开始，新行长度 row+1。
2. 先用1初始化两端及所有位置。
3. 对内部 col=1..row-1，根据上一行两邻项赋值。
4. 把完整新行移动进输出。

## 手工追踪

已有 [1,3,3,1] 时生成下一行：先 [1,1,1,1,1]，内部依次1+3=4、3+3=6、3+1=4，得到 [1,4,6,4,1]。两端没有两个父节点，必须保留1。

## 完整 C++17 实现

以下内容来自本课 [可编译头文件](../../../algorithms/cpp/solutions/p0118_pascals_triangle.hpp)，namespace用于让100题可同时进入同一工程。在线提交时一般只保留评测要求的类声明，并按平台要求处理标准库命名；本地辅助头文件负责统一导入标准库。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0118 {
class Solution {
public:
    std::vector<std::vector<int>> generate(int numRows) {
        std::vector<std::vector<int>> triangle;
        for (int row = 0; row < numRows; ++row) {
            std::vector<int> values(row + 1, 1);
            for (int col = 1; col < row; ++col)
                values[col] = triangle.back()[col-1] + triangle.back()[col];
            triangle.push_back(std::move(values));
        }
        return triangle;
    }
};
} // namespace hot100::p0118
```

## 复杂度核算

时间 Θ(r²)，输出包含 r(r+1)/2 项，已达输出下界。输出空间 Θ(r²)；构建新行的临时工作空间 O(r)，移动后归属输出；无递归。

## C++ 常见陷阱

- row=0时 triangle.back() 不存在，但内部循环为空，避免访问；不要把 back 取到循环外。
- 上一行引用在 triangle.push_back 扩容后可能失效，本实现完成所有读取后才压入。
- 推广到大行数需要大整数；先用 long long 加法也只能延缓溢出。

## 测试与运行

[完整测试源文件](../../../algorithms/cpp/tests/test_p0118.cpp)：验证1到30行的每一项与独立组合数递推一致，同时核对行长和行和2^r；含0行和完整小三角。

从仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0118.cpp -o /tmp/hot100-cpp/test_p0118
/tmp/hot100-cpp/test_p0118
# 环境支持时增加运行期检查：
g++ -std=c++17 -Wall -Wextra -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0118.cpp -o /tmp/hot100-cpp/test_p0118_san
ASAN_OPTIONS=detect_leaks=0 /tmp/hot100-cpp/test_p0118_san
```

本次环境的LeakSanitizer受ptrace限制，因此命令显式关闭泄漏检测；AddressSanitizer与UndefinedBehaviorSanitizer仍启用。支持泄漏检测的本地环境可移除该环境变量。断言通过说明覆盖输入上的本地结果正确，不能替代证明、完整平台约束核对或在线评测。

## 面试追问

- 只求第k行时，如何从右向左原地更新降低空间？
- 杨辉三角每行之和为何是2的行号次方？

## 练习与提示

- 练习1：实现只返回第k行的 O(k) 空间版。提示：内部从右向左加，防止新值污染旧值。
- 练习2：用组合数递推验证每一项。提示：C(r,c+1)=C(r,c)*(r-c)/(c+1)，测试时先扩大乘法类型。
