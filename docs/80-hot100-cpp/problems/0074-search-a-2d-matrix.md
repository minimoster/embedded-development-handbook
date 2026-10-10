# 74. 搜索二维矩阵：C++17 推导与实现

> 分类：二分查找 · 难度：MEDIUM · 官方 Hot 100 顺序：64/100

## 题意与调用约定

矩阵各行递增，且下一行首元素大于上一行末元素。判断目标是否存在；矩阵是规则矩形。与第240题不同，这里整张矩阵按行展开后有全局顺序。

题面与最新约束见 [力扣中文官方题目](https://leetcode.cn/problems/search-a-2d-matrix/) / [LeetCode 官方英文题目](https://leetcode.com/problems/search-a-2d-matrix/)。下文给出独立编写的推导、示例与实现；测试是本地验证，没有声称在线评测已通过。

## 从直觉到优化

遍历所有格子需要 O(mn)。既然行与行也互相有序，矩阵可被“虚拟展开”为一个长度 mn 的数组。无需真的分配展开数组：虚拟下标 k 对应第 k/cols 行、第 k%cols 列。对这个序列做 lower_bound 即可。

## 不变量与正确性

展平映射保持元素顺序且是一一对应。二分中 [0,lo) 的值均小于 target，[hi,mn) 的值均不小于 target；退出后只需检验分界值是否等于目标。矩阵访问的下标都由小于 mn 的虚拟下标产生。

## 算法步骤

1. 先处理零行或零列，避免除以零。
2. 以 [0,rows*cols) 为候选区间。
3. 中点通过除法和取余映射回行列，再按普通二分缩区间。
4. 退出时先检查 lo<总长度，再访问对应格子。

## 手工追踪

矩阵=[[-5,-2,1],[4,8,12]]，cols=3，target=4。虚拟 mid=3 对应(1,0)，值4，使 hi=3；随后 mid=1 值-2，lo=2；mid=2 值1，lo=3。分界3正好命中4。

## 完整 C++17 实现

以下内容来自本课 [可编译头文件](../../../algorithms/cpp/solutions/p0074_search_a_2d_matrix.hpp)，namespace用于让100题可同时进入同一工程。在线提交时一般只保留评测要求的类声明，并按平台要求处理标准库命名；本地辅助头文件负责统一导入标准库。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0074 {
class Solution {
public:
    bool searchMatrix(std::vector<std::vector<int>>& matrix, int target) {
        if (matrix.empty() || matrix.front().empty()) return false;
        std::size_t cols = matrix.front().size();
        std::size_t lo = 0, hi = matrix.size() * cols;
        while (lo < hi) {
            std::size_t mid = lo + (hi - lo) / 2;
            int value = matrix[mid / cols][mid % cols];
            if (value < target) lo = mid + 1;
            else hi = mid;
        }
        return lo < matrix.size() * cols && matrix[lo / cols][lo % cols] == target;
    }
};
} // namespace hot100::p0074
```

## 复杂度核算

时间 O(log(mn+1))；额外空间 O(1)。虚拟展平不创建 O(mn) 的副本。

## C++ 常见陷阱

- 使用 size_t 承接容器长度；但 rows*cols 仍须能在 size_t 内表示。
- 短路与表达式保证尾后位置不被索引；不可交换最终条件的先后顺序。
- 若输入行长不一致或仅每行每列有序，本证明失效；别直接复用到第240题。

## 测试与运行

[完整测试源文件](../../../algorithms/cpp/tests/test_p0074.cpp)：遍历1到7行/列规则矩阵及大量命中/缺失目标，与逐格扫描对拍；含零行、零列和跨行边界。

从仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0074.cpp -o /tmp/hot100-cpp/test_p0074
/tmp/hot100-cpp/test_p0074
# 环境支持时增加运行期检查：
g++ -std=c++17 -Wall -Wextra -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0074.cpp -o /tmp/hot100-cpp/test_p0074_san
ASAN_OPTIONS=detect_leaks=0 /tmp/hot100-cpp/test_p0074_san
```

本次环境的LeakSanitizer受ptrace限制，因此命令显式关闭泄漏检测；AddressSanitizer与UndefinedBehaviorSanitizer仍启用。支持泄漏检测的本地环境可移除该环境变量。断言通过说明覆盖输入上的本地结果正确，不能替代证明、完整平台约束核对或在线评测。

## 面试追问

- 比较“先找行再找列”与虚拟展开，时间上有什么关系？
- 若数据存在外存，每次随机访问很贵，应如何调整搜索粒度？

## 练习与提示

- 练习1：返回目标的二维坐标。提示：命中后把虚拟位置除以列数并取余。
- 练习2：构造每行每列递增但展开非递增的反例。提示：两行区间允许交叠。

## 实现复用与边界检查

先逐行选择目标行再二分的成本是 O(m+log n)。阶梯搜索为 O(m+n)，也不同于本题对数时间目标；此实现采用虚拟展开二分。
