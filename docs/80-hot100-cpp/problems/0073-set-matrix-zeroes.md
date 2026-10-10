# 73. 矩阵置零（C++17）

> 矩阵 · MEDIUM。前置：能读懂本题使用的容器与递归/循环状态。建议先手推，再自己写，最后看实现。

## 题意与接口边界

矩形矩阵中原来出现0的行和列最终全部置0；新置的0不能继续传播到别的行列。原地操作。

题目与完整输入约束：[官方题页](https://leetcode.cn/problems/set-matrix-zeroes/)。本页为原创教学表达，不重复完整官方题面。

## 从朴素思路走向主解

立即发现0就整行列置0会污染后续判断。先记录原0的行列，再统一修改。通常要两个标记数组；第一行和第一列恰可借作它们，但它们自己的原始状态会与标记混在一起，所以另存两个布尔。

## 状态与执行步骤

先检查原第一行/列是否有0；只扫描内部元素，遇原0时标记matrix[i][0]和matrix[0][j]；第二遍依据标记置内部；最后根据两个布尔处理第一行/列。

## 手推一组自建例子

[[1,2,3],[4,0,6],[7,8,9]] 先在首列行1与首行列1写标记0。内部第二遍清行1、列1，得到中间十字0；第一行原本无0，除被列标记的中间位外不整体清零。

读到这里先暂停：不看代码，画出每次选择/指针移动后的状态；必须能解释为什么这一步安全，才算理解。

## 为什么正确

写标记只改变边缘，不会影响尚未扫描的内部原值。内部修改仅依据完整原始零信息；首行/首列原零情况通过独立布尔保存。因此结果恰好对应原始0触发的行列并集，没有连锁扩大。

## 完整 C++17 实现

源码：[可编译头文件](../../../algorithms/cpp/solutions/p0073_set_matrix_zeroes.hpp)。本地包装的namespace用于把100题同时编译；在线平台提交时只保留类及所需标准头，链表/树节点以平台提供定义为准。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0073 {
class Solution {
public:
    void setZeroes(std::vector<std::vector<int>>& matrix) {
        if (matrix.empty() || matrix[0].empty()) return;
        const int rows = static_cast<int>(matrix.size()), cols = static_cast<int>(matrix[0].size());
        bool firstRow = false, firstCol = false;
        for (int j = 0; j < cols; ++j) if (matrix[0][j] == 0) firstRow = true;
        for (int i = 0; i < rows; ++i) if (matrix[i][0] == 0) firstCol = true;
        for (int i = 1; i < rows; ++i) for (int j = 1; j < cols; ++j)
            if (matrix[i][j] == 0) matrix[i][0] = matrix[0][j] = 0;
        for (int i = 1; i < rows; ++i) for (int j = 1; j < cols; ++j)
            if (matrix[i][0] == 0 || matrix[0][j] == 0) matrix[i][j] = 0;
        if (firstRow) std::fill(matrix[0].begin(), matrix[0].end(), 0);
        if (firstCol) for (int i = 0; i < rows; ++i) matrix[i][0] = 0;
    }
};
} // namespace hot100::p0073
```

## 复杂度与存储边界

时间 O(mn)，额外空间 O(1)。第一行列预扫描是O(m+n)，被主扫描上界包含；矩阵本身就地修改。

## C++与边界陷阱

第一行和第一列共享matrix[0][0]，不能单独靠它表示两个状态。先清第一行会破坏列标记，故边缘处理放最后。vector<vector<int>>理论可不规则，本题要求矩形，本实现不为ragged输入兜底。

## 面试追问：先说变化，再改算法

如果matrix不可写，O(m+n)标记数组更适合。嵌入式里共享存储作元数据要明确恢复/覆盖顺序，这题可用来练习“先记状态，再批量更新”的事务思想。

## 自测与迁移练习

练习1：只用一个bool加matrix[0][0]压缩两个标记，提示必须设计可靠遍历顺序。练习2：实现不可变输入版本并对拍，提示从原矩阵搜0，不能从已修改结果再推原标记。

本题[可运行测试](../../../algorithms/cpp/tests/test_p0073.cpp)包含普通、边界或重复调用用例；有适合独立oracle的题还做固定种子的随机/穷举对拍。测试不是在线判题AC记录。仓库根目录可单独运行：

```sh
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0073.cpp -o /tmp/hot100-p0073
/tmp/hot100-p0073
```

复习门槛：能用自己的话复述状态含义，手推一组未见输入，指出复杂度包含哪些存储，再闭卷实现并通过测试。只记模板而说不出不变式，还需要回到手推。
