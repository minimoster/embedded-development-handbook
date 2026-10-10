# 11. 盛最多水的容器（C++17）

> 双指针 · MEDIUM。前置：能读懂本题使用的容器与递归/循环状态。建议先手推，再自己写，最后看实现。

## 题意与接口边界

选两条竖线，以水平距离乘较短高度作为面积，返回最大值。中间竖线不会影响所选容器容量。

题目与完整输入约束：[官方题页](https://leetcode.cn/problems/container-with-most-water/)。本页为原创教学表达，不重复完整官方题面。

## 从朴素思路走向主解

枚举两端是 O(n²)。从最大宽度出发，每移动一步宽度都会缩小，只有改善短板才可能补偿宽度损失。因此每次舍弃较短的一端，而不是朝较高的一端移动。

## 状态与执行步骤

初始化两端。计算当前面积并更新best；左不高于右时left++，否则right--；两端相遇就结束。面积乘法先提升到long long，本题结果在官方范围内再转int。

## 手推一组自建例子

[2,6,3,5,4]：两端面积4×2=8，移左到6后宽3、短板4，面积12；此时移右到5，宽2、短板5，面积10；之后宽更小，最佳仍12。

读到这里先暂停：不看代码，画出每次选择/指针移动后的状态；必须能解释为什么这一步安全，才算理解。

## 为什么正确

若左端较短，固定左端并把右端向内移，宽度减小而短板至多仍为左高，所以不可能超过已记录面积。丢弃左端不会损失未记录更优答案。右端较短时同理，构成每一步的淘汰证明。

## 完整 C++17 实现

源码：[可编译头文件](../../../algorithms/cpp/solutions/p0011_container_with_most_water.hpp)。本地包装的namespace用于把100题同时编译；在线平台提交时只保留类及所需标准头，链表/树节点以平台提供定义为准。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0011 {
class Solution {
public:
    int maxArea(std::vector<int>& height) {
        int left = 0, right = static_cast<int>(height.size()) - 1;
        long long best = 0;
        while (left < right) {
            best = std::max(best, 1LL * (right - left) * std::min(height[left], height[right]));
            if (height[left] <= height[right]) ++left;
            else --right;
        }
        return static_cast<int>(best); // Fits the official result bounds.
    }
};
} // namespace hot100::p0011
```

## 复杂度与存储边界

时间 O(n)，额外空间 O(1)。每次至少移动一个端点，至多 n-1 次比较。

## C++与边界陷阱

面积不是两端高度的平均，也不是高度差。等高时移动任一端都正确，要说清淘汰依据。扩展到巨大数组时 int 返回值可能放不下，不能以中间用long long就声称任意输入安全。

## 面试追问：先说变化，再改算法

为什么不能移动较高端？请给出反例并复述短端上界。若要求返回端点下标，更新best时保存它们；平局规则应显式约定。

## 自测与迁移练习

练习1：实现暴力参考解并对拍随机非负高度，提示不要用同一双指针作为oracle。练习2：找一组移动高端失败的例子，提示让低端内侧有高柱，而高端当前已足够高。

本题[可运行测试](../../../algorithms/cpp/tests/test_p0011.cpp)包含普通、边界或重复调用用例；有适合独立oracle的题还做固定种子的随机/穷举对拍。测试不是在线判题AC记录。仓库根目录可单独运行：

```sh
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0011.cpp -o /tmp/hot100-p0011
/tmp/hot100-p0011
```

复习门槛：能用自己的话复述状态含义，手推一组未见输入，指出复杂度包含哪些存储，再闭卷实现并通过测试。只记模板而说不出不变式，还需要回到手推。
