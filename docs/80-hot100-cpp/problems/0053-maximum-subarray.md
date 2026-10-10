# 53. 最大子数组和（C++17）

> 普通数组 · MEDIUM。前置：能读懂本题使用的容器与递归/循环状态。建议先手推，再自己写，最后看实现。

## 题意与接口边界

求和最大的非空连续子数组。官方数组非空；本地对空数组扩展返回0，但不以空区间替代全负数组的答案。

题目与完整输入约束：[官方题页](https://leetcode.cn/problems/maximum-subarray/)。本页为原创教学表达，不重复完整官方题面。

## 从朴素思路走向主解

枚举区间起点和终点会重复计算。设ending是以当前位置结尾的最优和，新数x加入时只有两种选择：接在旧区间后，或从x重新开始。历史最优best另外保存，不能用ending最终值代替它。

## 状态与执行步骤

用第一个数初始化ending和best；逐个算 ending=max(x,ending+x)；然后best=max(best,ending)。每步只需要上一位置状态，故无需整张dp表。

## 手推一组自建例子

[-4,5,-2,3,-6] 的ending依次-4、5、3、6、0；best依次-4、5、5、6、6。最佳区间是[5,-2,3]；最后ending为0不代表全局答案0。

读到这里先暂停：不看代码，画出每次选择/指针移动后的状态；必须能解释为什么这一步安全，才算理解。

## 为什么正确

任何以i结尾的非空区间，要么只含nums[i]，要么是某个以i-1结尾区间再追加nums[i]；选其中最优即可。归纳得到ending定义成立，再对所有终点取最大就是所有非空区间的最大和。

## 完整 C++17 实现

源码：[可编译头文件](../../../algorithms/cpp/solutions/p0053_maximum_subarray.hpp)。本地包装的namespace用于把100题同时编译；在线平台提交时只保留类及所需标准头，链表/树节点以平台提供定义为准。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0053 {
class Solution {
public:
    int maxSubArray(std::vector<int>& nums) {
        if (nums.empty()) return 0;
        long long ending = nums[0], best = nums[0];
        for (std::size_t i = 1; i < nums.size(); ++i) {
            ending = std::max(static_cast<long long>(nums[i]), ending + nums[i]);
            best = std::max(best, ending);
        }
        return static_cast<int>(best);
    }
};
} // namespace hot100::p0053
```

## 复杂度与存储边界

时间 O(n)，额外空间 O(1)。long long存状态避免中间加法溢出；转int仍依赖官方最大结果范围，若作为通用API需改返回类型。

## C++与边界陷阱

best初始化为0会把全负输入误判成0。递推之前更新best会遗漏当前更好区间。题目允许负数，不能假设越长和越大。

## 面试追问：先说变化，再改算法

同时返回区间下标时，重启ending要记录新起点，best改善时保存端点。环形最大子数组可考虑总和减最小子数组，但全负情形需要独立处理。

## 自测与迁移练习

练习1：返回最优区间，平局选最短，提示比较和时同时比较长度。练习2：解释为什么“删掉所有负数相加”求的是子序列而非子数组，给一个负数桥接两个大正数的例子。

本题[可运行测试](../../../algorithms/cpp/tests/test_p0053.cpp)包含普通、边界或重复调用用例；有适合独立oracle的题还做固定种子的随机/穷举对拍。测试不是在线判题AC记录。仓库根目录可单独运行：

```sh
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0053.cpp -o /tmp/hot100-p0053
/tmp/hot100-p0053
```

复习门槛：能用自己的话复述状态含义，手推一组未见输入，指出复杂度包含哪些存储，再闭卷实现并通过测试。只记模板而说不出不变式，还需要回到手推。
