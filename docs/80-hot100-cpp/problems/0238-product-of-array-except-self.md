# 238. 除了自身以外数组的乘积（C++17）

> 普通数组 · MEDIUM。前置：能读懂本题使用的容器与递归/循环状态。建议先手推，再自己写，最后看实现。

## 题意与接口边界

返回每个位置之外所有元素的乘积，不使用除法。官方保证相关前后缀乘积及结果满足32位整数范围。

题目与完整输入约束：[官方题页](https://leetcode.cn/problems/product-of-array-except-self/)。本页为原创教学表达，不重复完整官方题面。

## 从朴素思路走向主解

总积除当前值在0出现时失败。把“除自己以外”拆成左边所有数的积乘右边所有数的积。输出数组先承载左积，再以一个变量携带右积，可省去第二个辅助数组。

## 状态与执行步骤

从左到右：先把当前prefix写入answer[i]，再prefix*=nums[i]。从右到左：answer[i]*=suffix，再suffix*=nums[i]。空乘积定义为1，故两端不需要特判。

## 手推一组自建例子

[2,3,4]第一遍answer=[1,2,6]；右向遍历：i2乘1得6，suffix=4；i1乘4得8，suffix=12；i0乘12得12。一个0时只有0位置可能非0，两个0时所有答案为0，自然由乘法得出。

读到这里先暂停：不看代码，画出每次选择/指针移动后的状态；必须能解释为什么这一步安全，才算理解。

## 为什么正确

第一遍到i前prefix正是下标<i的积；第二遍到i前suffix正是下标>i的积。两部分不重叠且遗漏恰好i，所以其乘积满足题意。先使用再更新避免把当前数混入其中。

## 完整 C++17 实现

源码：[可编译头文件](../../../algorithms/cpp/solutions/p0238_product_of_array_except_self.hpp)。本地包装的namespace用于把100题同时编译；在线平台提交时只保留类及所需标准头，链表/树节点以平台提供定义为准。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0238 {
class Solution {
public:
    std::vector<int> productExceptSelf(std::vector<int>& nums) {
        std::vector<int> answer(nums.size(), 1);
        long long prefix = 1;
        for (std::size_t i = 0; i < nums.size(); ++i) {
            answer[i] = static_cast<int>(prefix);
            prefix *= nums[i];
        }
        long long suffix = 1;
        for (int i = static_cast<int>(nums.size()) - 1; i >= 0; --i) {
            answer[i] = static_cast<int>(answer[i] * suffix);
            suffix *= nums[i];
        }
        return answer;
    }
};
} // namespace hot100::p0238
```

## 复杂度与存储边界

时间 O(n)；除输出外额外O(1)，若计输出则O(n)。long long处理中间乘法但仍只承诺官方范围；任意长度、任意int的乘积可能超过long long。

## C++与边界陷阱

把prefix先乘nums[i]再存，会包含自己。不能只宣称“O(1)空间”而不说明排除了必需输出。负数与0均无需分支，除法法反而增加边界逻辑。

## 面试追问：先说变化，再改算法

若要模M乘积，无除法版本仍可用，不要求M是质数。若数据不断修改并查询除某元素积，可考虑线段树维护区间积，但0与数值范围仍要定义。

## 自测与迁移练习

练习1：推广为每个位置之外元素的和，提示加法版本可总和减当前值但要防溢出。练习2：在模1,000,000,007下求答案，提示每次乘法取模并先用64位；不能直接拿模逆处理0。

本题[可运行测试](../../../algorithms/cpp/tests/test_p0238.cpp)包含普通、边界或重复调用用例；有适合独立oracle的题还做固定种子的随机/穷举对拍。测试不是在线判题AC记录。仓库根目录可单独运行：

```sh
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0238.cpp -o /tmp/hot100-p0238
/tmp/hot100-p0238
```

复习门槛：能用自己的话复述状态含义，手推一组未见输入，指出复杂度包含哪些存储，再闭卷实现并通过测试。只记模板而说不出不变式，还需要回到手推。
