# 128. 最长连续序列（C++17）

> 哈希 · MEDIUM。前置：能读懂本题使用的容器与递归/循环状态。建议先手推，再自己写，最后看实现。

## 题意与接口边界

找数值连续的一段集合的最大长度，元素在原数组里不必相邻，重复值只算一次。

题目与完整输入约束：[官方题页](https://leetcode.cn/problems/longest-consecutive-sequence/)。本页为原创教学表达，不重复完整官方题面。

## 从朴素思路走向主解

排序能做到 O(n log n)，但题目希望平均线性。集合提供快速存在性检查，关键并非从每个数都向右数，而是只从没有前驱的数开始；否则一个长连续段会反复扫描。

## 状态与执行步骤

先用集合去重。枚举 start，若 start-1 存在便跳过；否则沿 start+1、start+2 计数，更新最长长度。对 INT_MIN/INT_MAX 先做边界判断再加减。

## 手推一组自建例子

[8,3,4,2,3,9] 去重得{2,3,4,8,9}。3、4、9有前驱，不开新扫描。2扫描2→3→4得3；8扫描8→9得2；答案3。

读到这里先暂停：不看代码，画出每次选择/指针移动后的状态；必须能解释为什么这一步安全，才算理解。

## 为什么正确

每个有限连续段有唯一最小元素，只有它通过无前驱检查。从此起点的扫描恰好覆盖该段。不同段不重叠，所以内层循环总步数至多集合元素个数，不能机械地把双层循环写成 O(n²)。

## 完整 C++17 实现

源码：[可编译头文件](../../../algorithms/cpp/solutions/p0128_longest_consecutive_sequence.hpp)。本地包装的namespace用于把100题同时编译；在线平台提交时只保留类及所需标准头，链表/树节点以平台提供定义为准。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0128 {
class Solution {
public:
    int longestConsecutive(std::vector<int>& nums) {
        std::unordered_set<int> values(nums.begin(), nums.end());
        int best = 0;
        for (int start : values) {
            if (start != INT_MIN && values.count(start - 1)) continue;
            int current = start, length = 1;
            while (current != INT_MAX && values.count(current + 1)) {
                ++current; ++length;
            }
            best = std::max(best, length);
        }
        return best;
    }
};
} // namespace hot100::p0128
```

## 复杂度与存储边界

均摊 O(n) 时间、O(n) 空间。哈希集合的恶意碰撞可使最坏时间退化；题目常用哈希模型下的线性，不等于所有哈希实现的确定性线性。

## C++与边界陷阱

先去重再遍历集合，避免重复起点触发重复扫描。直接 num-1、++cur_num 的写法应结合输入数值范围理解；可复用库在int全范围要防溢出，故本实现加了端点守卫。

## 面试追问：先说变化，再改算法

数据流中不断插入新值时，端点区间长度哈希能在线合并左右区间，但重复值处理不可省。若要求返回具体区间，同时记录最优起点即可。

## 自测与迁移练习

练习1：返回最长段的起点与终点，提示平局先明确规则。练习2：删掉无前驱判断并用长连续数组测运行量，提示统计集合查找次数而不是只看代码嵌套层数。

本题[可运行测试](../../../algorithms/cpp/tests/test_p0128.cpp)包含普通、边界或重复调用用例；有适合独立oracle的题还做固定种子的随机/穷举对拍。测试不是在线判题AC记录。仓库根目录可单独运行：

```sh
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0128.cpp -o /tmp/hot100-p0128
/tmp/hot100-p0128
```

复习门槛：能用自己的话复述状态含义，手推一组未见输入，指出复杂度包含哪些存储，再闭卷实现并通过测试。只记模板而说不出不变式，还需要回到手推。
