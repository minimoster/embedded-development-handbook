# 131. 分割回文串（C++17）

> 回溯 · MEDIUM。前置：能读懂本题使用的容器与递归/循环状态。建议先手推，再自己写，最后看实现。

## 题意与接口边界

把字符串完全分成若干非空连续片段，每片都为回文，输出全部不同切分。片段顺序必须与原串一致。

题目与完整输入约束：[官方题页](https://leetcode.cn/problems/palindrome-partitioning/)。本页为原创教学表达，不重复完整官方题面。

## 从朴素思路走向主解

切分可视为逐个选择下一个结束位置，每次都两端判回文会重复检查同样区间。先用区间DP缓存s[l..r]是否回文，再回溯切分，判合法边只需O(1)。

## 状态与执行步骤

倒序枚举left、正序right，pal[l][r]由两端相同且内部回文决定，长度≤2直接判断两端。visit(start)枚举所有合法end，追加substring并递归end+1；start==n输出完整切分。

## 手推一组自建例子

“aba”从0出发：选a，后面b、a各自回文，得[a,b,a]；选ab被DP判false而剪掉；选aba回文，得[aba]。回文表pal[0][2]依赖pal[1][1]，故先算内层。

读到这里先暂停：不看代码，画出每次选择/指针移动后的状态；必须能解释为什么这一步安全，才算理解。

## 为什么正确

回文递推按区间长度等价正确，倒序left保证pal[left+1][right-1]已求出。DFS路径覆盖恰好[0,start)且各片回文；选下一段后维持不变式。任意切分对应唯一结束下标序列，被全部枚举。

## 完整 C++17 实现

源码：[可编译头文件](../../../algorithms/cpp/solutions/p0131_palindrome_partitioning.hpp)。本地包装的namespace用于把100题同时编译；在线平台提交时只保留类及所需标准头，链表/树节点以平台提供定义为准。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0131 {
class Solution {
public:
    std::vector<std::vector<std::string>> partition(std::string s) {
        const int n = static_cast<int>(s.size());
        std::vector<std::vector<bool>> palindrome(n, std::vector<bool>(n, false));
        for (int left = n - 1; left >= 0; --left)
            for (int right = left; right < n; ++right)
                palindrome[left][right] = s[left] == s[right] && (right - left < 2 || palindrome[left + 1][right - 1]);
        std::vector<std::vector<std::string>> answer;
        std::vector<std::string> path;
        std::function<void(int)> visit = [&](int start) {
            if (start == n) { answer.push_back(path); return; }
            for (int end = start; end < n; ++end) if (palindrome[start][end]) {
                path.push_back(s.substr(start, end - start + 1));
                visit(end + 1); path.pop_back();
            }
        };
        visit(0); return answer;
    }
};
} // namespace hot100::p0131
```

## 复杂度与存储边界

预处理O(n²)时间/空间；最多2^(n-1)种切分，复制所有结果的字符总量O(n·2^n)，总体O(n²+n·2^n)。路径及栈O(n)，结果另外计；不要写纯O(n²)。

## C++与边界陷阱

短路条件right-left<2必须放在访问内部状态之前，避免left+1超范围。substr会复制，不是O(1)视图；只是回文判断O(1)。函数局部结果避免成员res跨调用残留。

## 面试追问：先说变化，再改算法

若只求最少回文片段，可在同一回文表上做一维最优切分DP。若输入巨大，输出数量主导瓶颈，再快的判回文也无法消除指数输出。

## 自测与迁移练习

练习1：只返回最少切割次数，提示dp[start]考虑所有合法end并取1+dp[end+1]，片段数和切割数相差1。练习2：用string_view减少内部复制，提示结果视图的生命周期必须不超过原串，返回接口需重新设计。

本题[可运行测试](../../../algorithms/cpp/tests/test_p0131.cpp)包含普通、边界或重复调用用例；有适合独立oracle的题还做固定种子的随机/穷举对拍。测试不是在线判题AC记录。仓库根目录可单独运行：

```sh
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0131.cpp -o /tmp/hot100-p0131
/tmp/hot100-p0131
```

复习门槛：能用自己的话复述状态含义，手推一组未见输入，指出复杂度包含哪些存储，再闭卷实现并通过测试。只记模板而说不出不变式，还需要回到手推。
