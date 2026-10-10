# 22. 括号生成（C++17）

> 回溯 · MEDIUM。前置：能读懂本题使用的容器与递归/循环状态。建议先手推，再自己写，最后看实现。

## 题意与接口边界

生成n对括号组成的全部合法字符串。合法意味着每个前缀左括号数不少于右括号数，最终相等。

题目与完整输入约束：[官方题页](https://leetcode.cn/problems/generate-parentheses/)。本页为原创教学表达，不重复完整官方题面。

## 从朴素思路走向主解

先枚举2^(2n)串再检查浪费在明显非法前缀上。回溯直接保持前缀合法：只要右括号数已与左括号数相等，就不能再放右括号；左括号数也不能超过n。

## 状态与执行步骤

状态opened、closed代表已放左右括号数。opened<n可放左括号；closed<opened可放右括号。closed达到n时，由不变式也有opened=n，路径完整，复制结果；每次选择后回退。

## 手推一组自建例子

n=2：根只能放(；第二步放(后只能连续两次)得到(())；退回第一层另一合法选择是)，得()，之后再()得()()。开头)与前缀())从未进入树。

读到这里先暂停：不看代码，画出每次选择/指针移动后的状态；必须能解释为什么这一步安全，才算理解。

## 为什么正确

递归保持0≤closed≤opened≤n，因而每个生成前缀合法；终点closed=n强制opened=n，字符串合法。任何合法括号串逐位都满足两个选择条件，故搜索可沿它走完；每个字符串选择序列唯一。

## 完整 C++17 实现

源码：[可编译头文件](../../../algorithms/cpp/solutions/p0022_generate_parentheses.hpp)。本地包装的namespace用于把100题同时编译；在线平台提交时只保留类及所需标准头，链表/树节点以平台提供定义为准。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0022 {
class Solution {
public:
    std::vector<std::string> generateParenthesis(int n) {
        std::vector<std::string> answer;
        std::string path;
        std::function<void(int,int)> visit = [&](int opened, int closed) {
            if (closed == n) { answer.push_back(path); return; }
            if (opened < n) { path.push_back('('); visit(opened + 1, closed); path.pop_back(); }
            if (closed < opened) { path.push_back(')'); visit(opened, closed + 1); path.pop_back(); }
        };
        if (n >= 0) visit(0, 0);
        return answer;
    }
};
} // namespace hot100::p0022
```

## 复杂度与存储边界

结果数为卡特兰数C_n，输出长度2n，时间O(n·C_n)，结果空间O(n·C_n)，辅助O(n)；C_n约4^n/(sqrt(pi)·n^(3/2))。不要把它等同于全部2^(2n)枚举。

## C++与边界陷阱

只检查最终左/右数相等会接受)(这类非法前缀。终止条件用closed==n依赖已有不变式，不能脱离它理解。结果局部化确保同一Solution多次调用不会积累旧答案。

## 面试追问：先说变化，再改算法

如何只计算合法串数而不列举？卡特兰递推或括号数状态DP。若括号有多种类型，除了数量还要维护类型栈，左右数量条件不再充分。

## 自测与迁移练习

练习1：计算n=4的结果数量但不生成字符串，提示dp[i]=sum(dp[j]*dp[i-1-j])。练习2：支持两种括号且总对数n，提示闭括号必须匹配栈顶类型，不能任意关一类。

本题[可运行测试](../../../algorithms/cpp/tests/test_p0022.cpp)包含普通、边界或重复调用用例；有适合独立oracle的题还做固定种子的随机/穷举对拍。测试不是在线判题AC记录。仓库根目录可单独运行：

```sh
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0022.cpp -o /tmp/hot100-p0022
/tmp/hot100-p0022
```

复习门槛：能用自己的话复述状态含义，手推一组未见输入，指出复杂度包含哪些存储，再闭卷实现并通过测试。只记模板而说不出不变式，还需要回到手推。
