# 46. 全排列（C++17）

> 回溯 · MEDIUM。前置：能读懂本题使用的容器与递归/循环状态。建议先手推，再自己写，最后看实现。

## 题意与接口边界

给互不相同的整数，生成全部排列。每个位置使用一个尚未选过的输入位置；本地把空输入的排列定义为一个空排列。

题目与完整输入约束：[官方题页](https://leetcode.cn/problems/permutations/)。本页为原创教学表达，不重复完整官方题面。

## 从朴素思路走向主解

排列是逐位决策树：第一位n种选择、第二位n-1种，直到全部用完。和子集的有序索引递进不同，排列下一位可以选任何未用位置；若限制只向右选会漏掉绝大多数排列。

## 状态与执行步骤

维护path和used。path长到n就复制进答案；否则枚举所有未用位置，标记并追加，递归，回来后弹出并取消标记。答案与状态都放在函数局部，重复调用不残留。

## 手推一组自建例子

[2,5,8]先选2，第二位可5或8，得到[2,5,8]、[2,8,5]；回到根选择5，得[5,2,8]、[5,8,2]；根选8再得两种。树中一条根到叶路径对应一个排列。

读到这里先暂停：不看代码，画出每次选择/指针移动后的状态；必须能解释为什么这一步安全，才算理解。

## 为什么正确

递归入口path中位置互不重复、used精确标记这些位置。每次扩展只选未用位置，叶子必为合法排列。任意排列的每个下一位都会被相应循环枚举，且选择序列唯一，所以所有排列恰出现一次。

## 完整 C++17 实现

源码：[可编译头文件](../../../algorithms/cpp/solutions/p0046_permutations.hpp)。本地包装的namespace用于把100题同时编译；在线平台提交时只保留类及所需标准头，链表/树节点以平台提供定义为准。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0046 {
class Solution {
public:
    std::vector<std::vector<int>> permute(std::vector<int>& nums) {
        std::vector<std::vector<int>> answer;
        std::vector<int> path;
        std::vector<bool> used(nums.size(), false);
        std::function<void()> visit = [&] {
            if (path.size() == nums.size()) { answer.push_back(path); return; }
            for (std::size_t i = 0; i < nums.size(); ++i) if (!used[i]) {
                used[i] = true; path.push_back(nums[i]);
                visit();
                path.pop_back(); used[i] = false;
            }
        };
        visit(); return answer;
    }
};
} // namespace hot100::p0046
```

## 复杂度与存储边界

输出 n!个长度n排列，时间至少 O(n·n!)，实现也在该阶；除结果外path、used、递归栈O(n)，输出O(n·n!)。不可把回溯空间只写成O(1)。

## C++与边界陷阱

回退时path.pop_back与used=false必须成对，遗漏会污染兄弟分支。官方元素互异，有重复时按位置枚举会得到相同值排列，需排序去重策略。std::function递归有封装开销但本题规模小且易读。

## 面试追问：先说变化，再改算法

原地swap生成排列可以省used，但仍需恢复交换且输出空间不变。全排列大到无法存储时，可改回调逐个消费；这不改变数量爆炸，只降低峰值结果存储。

## 自测与迁移练习

练习1：支持重复数字并只给不同排列，提示排序后当前重复值只有其前一个副本已用才允许使用。练习2：输出排列的前k个并安全停止，提示提前返回时也要完成状态回退。

本题[可运行测试](../../../algorithms/cpp/tests/test_p0046.cpp)包含普通、边界或重复调用用例；有适合独立oracle的题还做固定种子的随机/穷举对拍。测试不是在线判题AC记录。仓库根目录可单独运行：

```sh
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0046.cpp -o /tmp/hot100-p0046
/tmp/hot100-p0046
```

复习门槛：能用自己的话复述状态含义，手推一组未见输入，指出复杂度包含哪些存储，再闭卷实现并通过测试。只记模板而说不出不变式，还需要回到手推。
