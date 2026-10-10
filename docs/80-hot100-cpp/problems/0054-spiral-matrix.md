# 54. 螺旋矩阵（C++17）

> 矩阵 · MEDIUM。前置：能读懂本题使用的容器与递归/循环状态。建议先手推，再自己写，最后看实现。

## 题意与接口边界

按顺时针螺旋输出矩形矩阵所有元素，不改变矩阵。空矩阵本地返回空结果。

题目与完整输入约束：[官方题页](https://leetcode.cn/problems/spiral-matrix/)。本页为原创教学表达，不重复完整官方题面。

## 从朴素思路走向主解

模拟每步走向需要visited矩阵，边界法则把未访问区域始终表示成一个矩形。每走完一条边，立即缩掉该边，下一条边自然不会重复拐角。

## 状态与执行步骤

维护top/bottom/left/right。先上边从左到右，top++；右边从上到下，right--；如果还有行，走下边并bottom--；如果还有列，走左边并left++。之后继续内层。

## 手推一组自建例子

[[1,2,3],[4,5,6]]先上边1,2,3，top变1；右边6，right变1；下边5,4，bottom变0。此时行区间空，左边循环不执行，输出恰6项。

读到这里先暂停：不看代码，画出每次选择/指针移动后的状态；必须能解释为什么这一步安全，才算理解。

## 为什么正确

外层循环开始时未访问元素恰好在四个边界内。每条边遍历后移除对应边，不重复已经输出的角；退化为一行或一列时条件检查避免反向再读同一条边。逐层耗尽矩形，所有元素恰访问一次。

## 完整 C++17 实现

源码：[可编译头文件](../../../algorithms/cpp/solutions/p0054_spiral_matrix.hpp)。本地包装的namespace用于把100题同时编译；在线平台提交时只保留类及所需标准头，链表/树节点以平台提供定义为准。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0054 {
class Solution {
public:
    std::vector<int> spiralOrder(std::vector<std::vector<int>>& matrix) {
        std::vector<int> answer;
        if (matrix.empty() || matrix[0].empty()) return answer;
        int top = 0, bottom = static_cast<int>(matrix.size()) - 1;
        int left = 0, right = static_cast<int>(matrix[0].size()) - 1;
        while (top <= bottom && left <= right) {
            for (int j = left; j <= right; ++j) answer.push_back(matrix[top][j]);
            ++top;
            for (int i = top; i <= bottom; ++i) answer.push_back(matrix[i][right]);
            --right;
            if (top <= bottom) {
                for (int j = right; j >= left; --j) answer.push_back(matrix[bottom][j]);
                --bottom;
            }
            if (left <= right) {
                for (int i = bottom; i >= top; --i) answer.push_back(matrix[i][left]);
                ++left;
            }
        }
        return answer;
    }
};
} // namespace hot100::p0054
```

## 复杂度与存储边界

时间 O(mn)，除输出外O(1)空间；输出必须占O(mn)。边界变量用有符号int支持缩成-1这一终止状态。

## C++与边界陷阱

若无下边/左边前的边界检查，一行或一列会重复。矩阵必须矩形。不要将空判断放到读取matrix[0].size()之后。路径模拟oracle的visited实现独立于主解，可更有效发现边界错误。

## 面试追问：先说变化，再改算法

如果要求生成n×n螺旋矩阵，走边框的控制逻辑可复用，但写入递增数。若矩形存在洞，四边界假设失效，需要实际图遍历规则。

## 自测与迁移练习

练习1：从左下角开始逆时针输出，提示先写每条边方向与缩边顺序。练习2：统计1×n、n×1、2×2、3×3访问次数，提示不仅检查值集合，还检查顺序与是否重复。

本题[可运行测试](../../../algorithms/cpp/tests/test_p0054.cpp)包含普通、边界或重复调用用例；有适合独立oracle的题还做固定种子的随机/穷举对拍。测试不是在线判题AC记录。仓库根目录可单独运行：

```sh
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0054.cpp -o /tmp/hot100-p0054
/tmp/hot100-p0054
```

复习门槛：能用自己的话复述状态含义，手推一组未见输入，指出复杂度包含哪些存储，再闭卷实现并通过测试。只记模板而说不出不变式，还需要回到手推。
