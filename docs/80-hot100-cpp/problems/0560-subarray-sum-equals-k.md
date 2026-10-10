# 560. 和为 K 的子数组（C++17）

> 子串 · MEDIUM。前置：能读懂本题使用的容器与递归/循环状态。建议先手推，再自己写，最后看实现。

## 题意与接口边界

统计和等于k的非空连续子数组数量。允许负数和0，因此普通按和大小伸缩的窗口没有单调性。

题目与完整输入约束：[官方题页](https://leetcode.cn/problems/subarray-sum-equals-k/)。本页为原创教学表达，不重复完整官方题面。

## 从朴素思路走向主解

前缀和P[j]把区间和变为P[j]-P[i]。对当前P[j]，只要知道之前P[i]=P[j]-k出现多少次，便能一口气统计所有以此位置结束的解，不必枚举每个起点。

## 状态与执行步骤

把空前缀0记为出现1次。逐元素累加prefix，先查prefix-k的历史频次并加答案，再增加当前prefix频次。频率而非布尔存在性是关键。

## 手推一组自建例子

[2,-2,2]、k=2：空前缀{0:1}；P=2命中0得1；P=0未命中-2，0频次变2；P=2命中0得2，累计3。三个解是首个2、整个数组、末个2。

读到这里先暂停：不看代码，画出每次选择/指针移动后的状态；必须能解释为什么这一步安全，才算理解。

## 为什么正确

查询时表内只含严格更早前缀，故所计区间非空。每个解的右端唯一，扫描到该右端时左前缀被计一次。多个相同前缀代表多个不同起点，必须全部保留频次。

## 完整 C++17 实现

源码：[可编译头文件](../../../algorithms/cpp/solutions/p0560_subarray_sum_equals_k.hpp)。本地包装的namespace用于把100题同时编译；在线平台提交时只保留类及所需标准头，链表/树节点以平台提供定义为准。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0560 {
class Solution {
public:
    int subarraySum(std::vector<int>& nums, int k) {
        std::unordered_map<long long, int> frequency;
        frequency[0] = 1;
        long long prefix = 0, answer = 0;
        for (int x : nums) {
            prefix += x;
            auto it = frequency.find(prefix - k);
            if (it != frequency.end()) answer += it->second;
            ++frequency[prefix];
        }
        return static_cast<int>(answer);
    }
};
} // namespace hot100::p0560
```

## 复杂度与存储边界

均摊时间 O(n)、额外空间 O(n)。答案最坏 n(n+1)/2，返回int仅依据官方长度上界。中间前缀与答案用long long；哈希退化边界需区分。

## C++与边界陷阱

先插当前前缀再查询会在k=0时多算长度0区间。漏frequency[0]=1会漏从下标0开始的区间。不能用排序前缀简单两数和，因为必须保留前后顺序。

## 面试追问：先说变化，再改算法

若要最长而不是数量，只需保存每个前缀的最早位置；若求最短，保存最近位置。二维子矩阵和可固定行边界后压成一维，但复杂度另增一层。

## 自测与迁移练习

练习1：返回和为k的最长子数组长度，提示0前缀下标-1。练习2：解释[0,0,0]为何有6个解，用前缀频次1→2→3→4逐步算，不用背组合公式。

本题[可运行测试](../../../algorithms/cpp/tests/test_p0560.cpp)包含普通、边界或重复调用用例；有适合独立oracle的题还做固定种子的随机/穷举对拍。测试不是在线判题AC记录。仓库根目录可单独运行：

```sh
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0560.cpp -o /tmp/hot100-p0560
/tmp/hot100-p0560
```

复习门槛：能用自己的话复述状态含义，手推一组未见输入，指出复杂度包含哪些存储，再闭卷实现并通过测试。只记模板而说不出不变式，还需要回到手推。
