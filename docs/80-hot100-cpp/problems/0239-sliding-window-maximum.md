# 239. 滑动窗口最大值（C++17）

> 子串 · HARD。前置：能读懂本题使用的容器与递归/循环状态。建议先手推，再自己写，最后看实现。

## 题意与接口边界

给出每个长度k的连续窗口的最大值。窗口每次右移一格，只输出完整窗口。

题目与完整输入约束：[官方题页](https://leetcode.cn/problems/sliding-window-maximum/)。本页为原创教学表达，不重复完整官方题面。

## 从朴素思路走向主解

每窗扫描要 O(nk)，堆可 O(n log n)。单调双端队列只保留未来还有机会成为最大值的下标：新值更大且更晚到，旧小值从此永远不可能赢，可以删除。

## 状态与执行步骤

先从队首删过期下标；再从队尾删不大于新值的候选；压入i。形成完整窗口后，队首对应当前最大值。队列存下标，便于判断过期和访问原值。

## 手推一组自建例子

[4,2,2,7,1]、k=3：前两项队列[0,1]；第二个2替代旧2，首窗最大4。7来时0过期，旧2被7支配，队列只剩3；之后1跟在3后，两窗最大都为7。

读到这里先暂停：不看代码，画出每次选择/指针移动后的状态；必须能解释为什么这一步安全，才算理解。

## 为什么正确

队列下标递增、值严格递减，且全部位于当前窗口。被新值删掉的旧候选更小或相同而更早过期，在所有后续含旧值的窗口里新值也存在，因此删掉安全。队首大于其余候选，所有被删值又均被某候选支配，所以它是真正最大值。

## 完整 C++17 实现

源码：[可编译头文件](../../../algorithms/cpp/solutions/p0239_sliding_window_maximum.hpp)。本地包装的namespace用于把100题同时编译；在线平台提交时只保留类及所需标准头，链表/树节点以平台提供定义为准。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0239 {
class Solution {
public:
    std::vector<int> maxSlidingWindow(std::vector<int>& nums, int k) {
        std::vector<int> answer;
        if (k <= 0 || k > static_cast<int>(nums.size())) return answer;
        std::deque<int> candidates;
        for (int i = 0; i < static_cast<int>(nums.size()); ++i) {
            while (!candidates.empty() && candidates.front() <= i - k) candidates.pop_front();
            while (!candidates.empty() && nums[candidates.back()] <= nums[i]) candidates.pop_back();
            candidates.push_back(i);
            if (i + 1 >= k) answer.push_back(nums[candidates.front()]);
        }
        return answer;
    }
};
} // namespace hot100::p0239
```

## 复杂度与存储边界

时间 O(n)，空间 O(k)加输出 O(n-k+1)。每个下标入队一次、最多被弹一次；双while不能简单算成 O(nk)。

## C++与边界陷阱

过期条件是front<=i-k，不能写成<i-k。用<=删尾会留下相同最大值最新的位置，这使过期逻辑更简洁。vector.erase(begin())线性，不能冒充deque常数弹首。非法k本地返回空，官方k有效。

## 面试追问：先说变化，再改算法

求窗口最小值只需反转比较。若窗口长度动态改变，过期策略要重新分析；若要求中位数，单调队列不能维持中位数，需要有序结构或双堆等。

## 自测与迁移练习

练习1：保留相等尾值改成<，证明仍正确并比较队列长度。练习2：统计一组严格递增与严格递减数组的队列push/pop次数，提示用总入队/出队次数证明摊还界。

本题[可运行测试](../../../algorithms/cpp/tests/test_p0239.cpp)包含普通、边界或重复调用用例；有适合独立oracle的题还做固定种子的随机/穷举对拍。测试不是在线判题AC记录。仓库根目录可单独运行：

```sh
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0239.cpp -o /tmp/hot100-p0239
/tmp/hot100-p0239
```

复习门槛：能用自己的话复述状态含义，手推一组未见输入，指出复杂度包含哪些存储，再闭卷实现并通过测试。只记模板而说不出不变式，还需要回到手推。
