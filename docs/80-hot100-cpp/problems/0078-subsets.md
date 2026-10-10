# 78. 子集（C++17）

> 回溯 · MEDIUM。前置：能读懂本题使用的容器与递归/循环状态。建议先手推，再自己写，最后看实现。

## 题意与接口边界

输出互异整数数组的所有子集，包括空集。一个子集内部不关心排列，本实现按输入位置递增保存元素。

题目与完整输入约束：[官方题页](https://leetcode.cn/problems/subsets/)。本页为原创教学表达，不重复完整官方题面。

## 从朴素思路走向主解

把排列的“所有未用位置”改为“只选之后的位置”，就能让同一集合只有一种递增选择路径。这里每个递归节点本身就是一个子集，不必等选满n才输出。

## 状态与执行步骤

visit(start)先记录当前path；再枚举i≥start，把nums[i]加入，递归到i+1，回退。根path为空，所以空集自然被输出；下标严格递增使每元素最多选一次。

## 手推一组自建例子

[2,4,7]根先报空集；选2报[2]，续4报[2,4]，续7报[2,4,7]；退回[2]再续7报[2,7]；根选4报[4]、[4,7]，最后[7]。共8个。

读到这里先暂停：不看代码，画出每次选择/指针移动后的状态；必须能解释为什么这一步安全，才算理解。

## 为什么正确

任何子集都可唯一表示为一组递增输入下标，递归恰好沿这组下标选下去。每节点输出一次，故覆盖全部子集且无重复。回退恢复path，使兄弟分支对应各自选择集合。

## 完整 C++17 实现

源码：[可编译头文件](../../../algorithms/cpp/solutions/p0078_subsets.hpp)。本地包装的namespace用于把100题同时编译；在线平台提交时只保留类及所需标准头，链表/树节点以平台提供定义为准。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0078 {
class Solution {
public:
    std::vector<std::vector<int>> subsets(std::vector<int>& nums) {
        std::vector<std::vector<int>> answer;
        std::vector<int> path;
        std::function<void(std::size_t)> visit = [&](std::size_t start) {
            answer.push_back(path);
            for (std::size_t i = start; i < nums.size(); ++i) {
                path.push_back(nums[i]); visit(i + 1); path.pop_back();
            }
        };
        visit(0); return answer;
    }
};
} // namespace hot100::p0078
```

## 复杂度与存储边界

n元素有2^n子集，总输出元素量n·2^(n-1)，时间O(n·2^n)、结果空间O(n·2^n)，辅助路径和栈O(n)。空集边界常数。

## C++与边界陷阱

把递归参数仍写i而非i+1会重复选择同一元素甚至无限递归；把i+1误写start+1可能得到重复或漏解。重复输入元素不在本题契约内，不能把互异假设藏起来。

## 面试追问：先说变化，再改算法

二进制mask枚举也可生成子集，n较小时清楚，但移位宽度和有符号溢出需要守卫。若只要大小k子集，可加入剩余元素不足的剪枝。

## 自测与迁移练习

练习1：仅生成大小k的子集，提示到k立即输出且停止深入。练习2：支持重复输入但输出唯一子集，提示同层跳过相同值，不同层仍可选第二个相同副本。

本题[可运行测试](../../../algorithms/cpp/tests/test_p0078.cpp)包含普通、边界或重复调用用例；有适合独立oracle的题还做固定种子的随机/穷举对拍。测试不是在线判题AC记录。仓库根目录可单独运行：

```sh
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0078.cpp -o /tmp/hot100-p0078
/tmp/hot100-p0078
```

复习门槛：能用自己的话复述状态含义，手推一组未见输入，指出复杂度包含哪些存储，再闭卷实现并通过测试。只记模板而说不出不变式，还需要回到手推。
