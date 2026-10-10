# 1. 两数之和（C++17）

> 哈希 · EASY。前置：能读懂本题使用的容器与递归/循环状态。建议先手推，再自己写，最后看实现。

## 题意与接口边界

返回两个不同位置，使对应元素相加等于目标。官方保证恰有一个答案；本地扩展对无解返回空数组，输入不变。

题目与完整输入约束：[官方题页](https://leetcode.cn/problems/two-sum/)。本页为原创教学表达，不重复完整官方题面。

## 从朴素思路走向主解

枚举所有下标对要 O(n²)。改成顺序问答：处理 x 时，之前是否见过 target-x？哈希表保存值到一个下标的映射，问题从双重循环变为单次扫描。重复值不是障碍，位置不同才是要求。

## 状态与执行步骤

先算补数并查询；命中就返回之前的位置与当前 i；未命中再记录当前值。顺序不能反过来，否则 target=2*x 时会把当前位置配给自己。

## 手推一组自建例子

自建例子 [4,9,6]，目标10。i=0查6失败，存4→0；i=1查1失败，存9→1；i=2查4命中0，返回[0,2]。对[5,5]目标10，第二个5找到第一个5。

读到这里先暂停：不看代码，画出每次选择/指针移动后的状态；必须能解释为什么这一步安全，才算理解。

## 为什么正确

循环开始时 seen 只包含下标小于 i 的元素。命中意味着两个位置不同且和正确。任意有效对的较晚元素被扫描时，较早元素一定已经记录，所以不会漏解。

## 完整 C++17 实现

源码：[可编译头文件](../../../algorithms/cpp/solutions/p0001_two_sum.hpp)。本地包装的namespace用于把100题同时编译；在线平台提交时只保留类及所需标准头，链表/树节点以平台提供定义为准。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0001 {
class Solution {
public:
    std::vector<int> twoSum(std::vector<int>& nums, int target) {
        std::unordered_map<long long, int> seen;
        for (int i = 0; i < static_cast<int>(nums.size()); ++i) {
            long long need = static_cast<long long>(target) - nums[i];
            auto found = seen.find(need);
            if (found != seen.end()) return {found->second, i};
            seen[nums[i]] = i;
        }
        return {}; // Defensive extension: official inputs guarantee one answer.
    }
};
} // namespace hot100::p0001
```

## 复杂度与存储边界

哈希均摊时间 O(n)，额外空间 O(n)；极端哈希退化时可到 O(n²)，这不是确定性线性保证。返回数组长度为2。

## C++与边界陷阱

target-nums[i] 用 long long 计算，避免有符号减法溢出。find 不会插入键；operator[] 用于写入。重复值覆盖旧下标不影响找任意解。若改成所有答案，应重新讨论重复位置及输出大小。

## 面试追问：先说变化，再改算法

不能用哈希表时可排序值与原下标，再做双指针，时间 O(n log n)。已经有序且只要求位置，可直接双指针 O(n)、O(1)空间。

## 自测与迁移练习

练习1：返回全部不同下标对，说明最坏输出可达 O(n²)，提示保存同值的所有下标。练习2：只判断存在性，提示返回类型可改bool，但查后存的不变式仍要保留。

本题[可运行测试](../../../algorithms/cpp/tests/test_p0001.cpp)包含普通、边界或重复调用用例；有适合独立oracle的题还做固定种子的随机/穷举对拍。测试不是在线判题AC记录。仓库根目录可单独运行：

```sh
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0001.cpp -o /tmp/hot100-p0001
/tmp/hot100-p0001
```

复习门槛：能用自己的话复述状态含义，手推一组未见输入，指出复杂度包含哪些存储，再闭卷实现并通过测试。只记模板而说不出不变式，还需要回到手推。
