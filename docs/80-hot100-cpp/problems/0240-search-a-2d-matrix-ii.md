# 240. 搜索二维矩阵 II（C++17）

> 矩阵 · MEDIUM。前置：能读懂本题使用的容器与递归/循环状态。建议先手推，再自己写，最后看实现。

## 题意与接口边界

矩阵各行和各列分别非递减，判断目标是否出现。没有承诺一行末尾小于下一行开头，不能把矩阵直接摊平后整体二分。

题目与完整输入约束：[官方题页](https://leetcode.cn/problems/search-a-2d-matrix-ii/)。本页为原创教学表达，不重复完整官方题面。

## 从朴素思路走向主解

右上角同时具备“该行最大”“该列最小”的特性：大于目标说明整列都太大，可以删列；小于目标说明整行都太小，可以删行。每一步都有一条整行/列的排除证据。

## 状态与执行步骤

从row=0、col=最后列开始；相等返回true；当前值大于target就col--；否则row++。走出矩阵说明所有候选都被排除，返回false。

## 手推一组自建例子

[[1,4,8],[2,5,9],[3,7,12]]找7：8太大，去掉末列；4太小，去掉首行；5太小，去掉第二行；来到7命中。找6则在5后到7、再左到3、最终出界。

读到这里先暂停：不看代码，画出每次选择/指针移动后的状态；必须能解释为什么这一步安全，才算理解。

## 为什么正确

剩余候选区域是当前行及下方、当前列及左方的矩形。当前值>target时该列以下均≥当前值，不可能是目标；当前值<target时该行左方均≤当前值，也不可能是目标。每次仅删除不可能区域，保持所有可能答案。

## 完整 C++17 实现

源码：[可编译头文件](../../../algorithms/cpp/solutions/p0240_search_a_2d_matrix_ii.hpp)。本地包装的namespace用于把100题同时编译；在线平台提交时只保留类及所需标准头，链表/树节点以平台提供定义为准。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0240 {
class Solution {
public:
    bool searchMatrix(std::vector<std::vector<int>>& matrix, int target) {
        if (matrix.empty() || matrix[0].empty()) return false;
        int row = 0, col = static_cast<int>(matrix[0].size()) - 1;
        while (row < static_cast<int>(matrix.size()) && col >= 0) {
            if (matrix[row][col] == target) return true;
            if (matrix[row][col] > target) --col;
            else ++row;
        }
        return false;
    }
};
} // namespace hot100::p0240
```

## 复杂度与存储边界

时间 O(m+n)，额外空间 O(1)。最多往下m次、向左n次；不是O(log mn)的二分搜索。

## C++与边界陷阱

左上角不具备两种相反方向的可淘汰性，不能照着同样判断乱走。col需有符号类型支持-1。空矩阵检查必须在读第0行前；矩形、单调条件是输入契约。

## 面试追问：先说变化，再改算法

与题74比较：74有全局行间顺序，可摊平成一维二分；240只有行列局部单调。极宽但很少行的矩阵也可每行二分，两种复杂度需按形状比较。

## 自测与迁移练习

练习1：从左下角推导对称解，提示大时向上、小时向右。练习2：构造行列都递增但摊平序列不递增的2×2矩阵，说明为何一维二分会失去前提。

本题[可运行测试](../../../algorithms/cpp/tests/test_p0240.cpp)包含普通、边界或重复调用用例；有适合独立oracle的题还做固定种子的随机/穷举对拍。测试不是在线判题AC记录。仓库根目录可单独运行：

```sh
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0240.cpp -o /tmp/hot100-p0240
/tmp/hot100-p0240
```

复习门槛：能用自己的话复述状态含义，手推一组未见输入，指出复杂度包含哪些存储，再闭卷实现并通过测试。只记模板而说不出不变式，还需要回到手推。
