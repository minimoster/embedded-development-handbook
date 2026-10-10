# 39. 组合总和（C++17）

> 回溯 · MEDIUM。前置：能读懂本题使用的容器与递归/循环状态。建议先手推，再自己写，最后看实现。

## 题意与接口边界

候选正整数互不重复，每个数可使用任意多次，输出和为target的不同值组合。输入排序后被修改；本地target=0定义为一个空组合。

题目与完整输入约束：[官方题页](https://leetcode.cn/problems/combination-sum/)。本页为原创教学表达，不重复完整官方题面。

## 从朴素思路走向主解

暴力枚举使用顺序会同时产生[3,7]与[7,3]。限定组合按非递减位置选择，可以唯一表示每种计数分配。与子集相比本题允许重复当前数，因此递归继续从i开始，而非i+1。

## 状态与执行步骤

先排序。visit(start,remaining)在剩余0时输出；枚举start及以后候选；遇大于remaining的数直接break，其后更大也无用；选择后递归(i,remaining-value)，回退。

## 手推一组自建例子

候选[3,5,7]目标10：以3开头，再选3后剩4，无进一步解；退回一个3后试7，得到[3,7]；以5开头可以再选5得[5,5]；以7开头余3但不能回到更小候选，因此不重复[7,3]。

读到这里先暂停：不看代码，画出每次选择/指针移动后的状态；必须能解释为什么这一步安全，才算理解。

## 为什么正确

递归中path下标非递减、其和加remaining等于原target。正候选保证remaining严格下降，搜索终止。任意合法组合排序后有唯一非递减序列，会被某条路径生成；禁止回到更早下标仅去除排列重复而不漏组合。

## 完整 C++17 实现

源码：[可编译头文件](../../../algorithms/cpp/solutions/p0039_combination_sum.hpp)。本地包装的namespace用于把100题同时编译；在线平台提交时只保留类及所需标准头，链表/树节点以平台提供定义为准。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0039 {
class Solution {
public:
    std::vector<std::vector<int>> combinationSum(std::vector<int>& candidates, int target) {
        std::sort(candidates.begin(), candidates.end());
        std::vector<std::vector<int>> answer;
        std::vector<int> path;
        std::function<void(std::size_t,int)> visit = [&](std::size_t start, int remaining) {
            if (remaining == 0) { answer.push_back(path); return; }
            for (std::size_t i = start; i < candidates.size(); ++i) {
                if (candidates[i] > remaining) break;
                path.push_back(candidates[i]);
                visit(i, remaining - candidates[i]);
                path.pop_back();
            }
        };
        if (target >= 0) visit(0, target);
        return answer;
    }
};
} // namespace hot100::p0039
```

## 复杂度与存储边界

令m为候选数、D=floor(target/minCandidate)，先排序O(m log m)。搜索成本写为O(访问搜索节点+尝试候选次数+输出总长度)最准确；宽松树界可写O(m log m+∑(d=0..D)m^d+输出总长度)，含每个节点一次终止候选检查。m=1时这个求和为D+1，不会错误地把一条长度D路径视为常数。辅助路径/栈O(D)，结果空间按实际输出计。不能无依据写O(2^m)，因元素可重复。

## C++与边界陷阱

必须是正候选；0会使remaining不下降导致无限递归，负数会打破剪枝与终止。官方排除它们，本解不接受任意整数候选。成员res重复调用残留属于可复用性风险，本实现用局部结果。

## 面试追问：先说变化，再改算法

变成每个候选只能用一次时递归参数改i+1，但有重复候选还要同层去重。只求组合数量可以做完全背包，遍历顺序决定计组合还是排列。

## 自测与迁移练习

练习1：每数只能用一次，手推参数i与i+1的差异。练习2：写数量DP并解释外层遍历候选为何避免重复排列，提示与外层遍历target比较一个小例子。

本题[可运行测试](../../../algorithms/cpp/tests/test_p0039.cpp)包含普通、边界或重复调用用例；有适合独立oracle的题还做固定种子的随机/穷举对拍。测试不是在线判题AC记录。仓库根目录可单独运行：

```sh
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0039.cpp -o /tmp/hot100-p0039
/tmp/hot100-p0039
```

复习门槛：能用自己的话复述状态含义，手推一组未见输入，指出复杂度包含哪些存储，再闭卷实现并通过测试。只记模板而说不出不变式，还需要回到手推。
