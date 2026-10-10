# 42. 接雨水（C++17）

> 双指针 · HARD。前置：能读懂本题使用的容器与递归/循环状态。建议先手推，再自己写，最后看实现。

## 题意与接口边界

各柱宽为1，水停在左右最高边界中较低的一侧，求所有柱上方水量之和。

题目与完整输入约束：[官方题页](https://leetcode.cn/problems/trapping-rain-water/)。本页为原创教学表达，不重复完整官方题面。

## 从朴素思路走向主解

对每个位置分别找左右最高柱是 O(n²)，预处理两份最高数组可降为线性时间。再观察：只要已知左侧最高不大于右侧最高，左端当前格的水位就已确定，无需知道右边更远处是否还有更高柱。

## 状态与执行步骤

维护未处理区间[left,right]与已见左/右最高。较小最高的一侧先处理：更新该侧最高后，加最高减当前柱高，推进端点。每格只处理一次，不会加负水量。

## 手推一组自建例子

[3,0,2,0,4]：先处理左端3，左最高3；右端4建立右最高4；接着左侧0、2、0分别贡献3、1、3，合计7。最高边界本身贡献0。

读到这里先暂停：不看代码，画出每次选择/指针移动后的状态；必须能解释为什么这一步安全，才算理解。

## 为什么正确

已处理部分的水量已经最终确定。若leftMax≤rightMax，右侧已存在不低于leftMax的柱；当前左柱不高于leftMax时，短板必为leftMax。若当前柱更高，更新后该格为新边界、贡献0。右侧对称成立。

## 完整 C++17 实现

源码：[可编译头文件](../../../algorithms/cpp/solutions/p0042_trapping_rain_water.hpp)。本地包装的namespace用于把100题同时编译；在线平台提交时只保留类及所需标准头，链表/树节点以平台提供定义为准。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0042 {
class Solution {
public:
    int trap(std::vector<int>& height) {
        int left = 0, right = static_cast<int>(height.size()) - 1;
        int leftMax = 0, rightMax = 0;
        long long water = 0;
        while (left <= right) {
            if (leftMax <= rightMax) {
                leftMax = std::max(leftMax, height[left]);
                water += leftMax - height[left++];
            } else {
                rightMax = std::max(rightMax, height[right]);
                water += rightMax - height[right--];
            }
        }
        return static_cast<int>(water);
    }
};
} // namespace hot100::p0042
```

## 复杂度与存储边界

时间 O(n)，额外空间 O(1)。long long累加中间值，最终返回int依赖官方水量范围。

## C++与边界陷阱

这是按每格算水，不是求一次容器面积。先更新当前侧最高再做减法。空数组使right=-1、循环不进入；不要用无符号right。非负高度是题意前提，负高度会改变物理模型。

## 面试追问：先说变化，再改算法

单调栈方案按洼地层次求水，适合解释宽度乘高度差；左右最高数组方案更易写出证明。现场先给出朴素公式，再决定优化路径，会比直接背双指针更清楚。

## 自测与迁移练习

练习1：实现两数组最高值参考解并对拍，提示每格 min(leftMax[i],rightMax[i])-height[i]。练习2：让每个柱有不同宽度，提示按格求水仍可乘当前格宽度，但坐标与栈宽度计算需另行定义。

本题[可运行测试](../../../algorithms/cpp/tests/test_p0042.cpp)包含普通、边界或重复调用用例；有适合独立oracle的题还做固定种子的随机/穷举对拍。测试不是在线判题AC记录。仓库根目录可单独运行：

```sh
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0042.cpp -o /tmp/hot100-p0042
/tmp/hot100-p0042
```

复习门槛：能用自己的话复述状态含义，手推一组未见输入，指出复杂度包含哪些存储，再闭卷实现并通过测试。只记模板而说不出不变式，还需要回到手推。
