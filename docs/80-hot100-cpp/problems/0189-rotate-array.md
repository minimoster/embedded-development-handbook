# 189. 轮转数组（C++17）

> 普通数组 · MEDIUM。前置：能读懂本题使用的容器与递归/循环状态。建议先手推，再自己写，最后看实现。

## 题意与接口边界

原地将数组向右轮转k步，k为非负；长度n时只需轮转k mod n步。

题目与完整输入约束：[官方题页](https://leetcode.cn/problems/rotate-array/)。本页为原创教学表达，不重复完整官方题面。

## 从朴素思路走向主解

额外数组可把原下标i送到(i+k)%n，但要O(n)空间。把原数组写成A+B，其中B是末尾k项，目标是B+A。整体反转得reverse(B)+reverse(A)，分别反转两段即可恢复它们内部顺序。

## 状态与执行步骤

空数组先返回以防模0；k取模；反转整个数组；反转前shift项；反转其余项。三次反转都使用半开迭代器区间，空段反转也合法。

## 手推一组自建例子

[1,2,3,4,5]、k=7等价于2。全反转[5,4,3,2,1]；前两项反转[4,5,3,2,1]；后三项反转[4,5,1,2,3]，正是末两项移到前面。

读到这里先暂停：不看代码，画出每次选择/指针移动后的状态；必须能解释为什么这一步安全，才算理解。

## 为什么正确

反转操作对拼接满足reverse(A+B)=reverse(B)+reverse(A)。第二、三次反转分别恢复B、A的内部顺序，只改变两段的相对顺序，因此每个元素最终位于正确位置。

## 完整 C++17 实现

源码：[可编译头文件](../../../algorithms/cpp/solutions/p0189_rotate_array.hpp)。本地包装的namespace用于把100题同时编译；在线平台提交时只保留类及所需标准头，链表/树节点以平台提供定义为准。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0189 {
class Solution {
public:
    void rotate(std::vector<int>& nums, int k) {
        if (nums.empty()) return;
        std::size_t shift = static_cast<std::size_t>(k) % nums.size();
        std::reverse(nums.begin(), nums.end());
        std::reverse(nums.begin(), nums.begin() + static_cast<std::ptrdiff_t>(shift));
        std::reverse(nums.begin() + static_cast<std::ptrdiff_t>(shift), nums.end());
    }
};
} // namespace hot100::p0189
```

## 复杂度与存储边界

时间 O(n)，额外空间 O(1)。三段反转的交换总量是线性，和k大小无关；模运算把巨大的k压到合法长度。

## C++与边界陷阱

必须先判空再做k%n。官方k非负，本实现没有定义负k左转语义；把负int转size_t再取模不会等价于数学的负轮转。iterator加法用ptrdiff_t兼容差值类型。

## 面试追问：先说变化，再改算法

环形替换法也O(1)空间，但索引循环的数量由gcd(n,k)决定，容易漏处理某个环。若传感器缓冲只需逻辑视图，维护头索引可避免实际搬移数组。

## 自测与迁移练习

练习1：支持有符号k，提示先算((k%n)+n)%n并用足够宽类型。练习2：实现逻辑环形视图，提示所有读取映射到原数组，不应在每次读取时真实rotate。

本题[可运行测试](../../../algorithms/cpp/tests/test_p0189.cpp)包含普通、边界或重复调用用例；有适合独立oracle的题还做固定种子的随机/穷举对拍。测试不是在线判题AC记录。仓库根目录可单独运行：

```sh
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0189.cpp -o /tmp/hot100-p0189
/tmp/hot100-p0189
```

复习门槛：能用自己的话复述状态含义，手推一组未见输入，指出复杂度包含哪些存储，再闭卷实现并通过测试。只记模板而说不出不变式，还需要回到手推。
