# 76. 最小覆盖子串（C++17）

> 子串 · HARD。前置：能读懂本题使用的容器与递归/循环状态。建议先手推，再自己写，最后看实现。

## 题意与接口边界

返回覆盖t所有字符及其重复次数的最短连续子串；无法覆盖返回空串。按字节计数，官方字母输入满足该语义。

题目与完整输入约束：[官方题页](https://leetcode.cn/problems/minimum-window-substring/)。本页为原创教学表达，不重复完整官方题面。

## 从朴素思路走向主解

固定长度窗口不适合未知答案长度。先向右扩到合法，再向左尽量缩。need表示还欠多少个对应字节，允许为负表示有富余；missing记录欠的总字符数，而非欠的字符种类。

## 状态与执行步骤

初始化t计数和missing。新字节到来，若仍欠它便missing--，随后need--。missing==0时记更短答案并移左；移走后need++，只有它重新变正才表示覆盖被破坏。

## 手推一组自建例子

s=cabbaa,t=aba，下标从0开始，right为当前已纳入窗口的下标；每个步骤的状态写在操作之后：

1. right=4、left=0：窗口cabba有a2、b2，missing=0，记录长度5。
2. 移走c后left=1、right=4：窗口abba仍覆盖，missing=0，记录长度4。
3. 移走首个a后left=2、right=4：窗口bba只有a1、b2，missing=1，停止收缩，必须先右扩。
4. 右扩到right=5、left=2：窗口bbaa有a2、b2，missing=0。长度4与已有最优相同，代码不更新平局答案。
5. 移走多余的首个b后left=3、right=5：窗口baa有a2、b1，missing=0，记录长度3。
6. 再移走b后left=4、right=5：窗口aa缺b，missing=1，停止收缩。最终返回先前记录的baa。

因此baa来自停止收缩后的再次右扩，不是把cabba直接一路缩到baa。

读到这里先暂停：不看代码，画出每次选择/指针移动后的状态；必须能解释为什么这一步安全，才算理解。

## 为什么正确

右扩只会增加可用字符，左缩只会减少。每次合法时连续左缩找到此right对应最短合法窗口；更左的起点要么更长，要么先前已记录。遍历所有right并取最小，得到全局最短。need与missing的更新精确区分多余字符和必需字符。

## 完整 C++17 实现

源码：[可编译头文件](../../../algorithms/cpp/solutions/p0076_minimum_window_substring.hpp)。本地包装的namespace用于把100题同时编译；在线平台提交时只保留类及所需标准头，链表/树节点以平台提供定义为准。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0076 {
class Solution {
public:
    std::string minWindow(std::string s, std::string t) {
        if (t.empty()) return "";
        std::array<int, 256> need{};
        for (unsigned char c : t) ++need[c];
        int missing = static_cast<int>(t.size()), left = 0;
        int bestStart = 0, bestLength = INT_MAX;
        for (int right = 0; right < static_cast<int>(s.size()); ++right) {
            unsigned char c = static_cast<unsigned char>(s[right]);
            if (need[c] > 0) --missing;
            --need[c];
            while (missing == 0) {
                if (right - left + 1 < bestLength) {
                    bestLength = right - left + 1; bestStart = left;
                }
                unsigned char out = static_cast<unsigned char>(s[left++]);
                ++need[out];
                if (need[out] > 0) ++missing;
            }
        }
        return bestLength == INT_MAX ? "" : s.substr(bestStart, bestLength);
    }
};
} // namespace hot100::p0076
```

## 复杂度与存储边界

时间 O(|s|+|t|)，固定计数算法状态空间O(1)，最终substr复制O(答案长度)。当前s、t按值接口另有O(|s|+|t|)输入副本；若计副本与输出，额外空间为O(|s|+|t|)，可用const std::string&消除入参副本。left/right都只前进，收缩循环累计至多|s|次。

## C++与边界陷阱

重复字符不能只用集合。“aba”要求两个a。need<0是合法富余，不能看见负数就纠错。char数组索引先转unsigned char。空t必须提前处理，避免missing=0导致空窗无限收缩越界。

## 面试追问：先说变化，再改算法

如果只要求出现每种字母而忽略频次，怎样修改初始化？如果是多流合并日志中找最小覆盖时间段，应先明确是否有统一顺序与窗口边界。

## 自测与迁移练习

练习1：返回最短窗的[left,right]而不复制字符串，提示无解用显式optional。练习2：手推t="aab"而s="aaacb"，逐步列need[a]与missing，验证富余a被移走时不改变合法性。

本题[可运行测试](../../../algorithms/cpp/tests/test_p0076.cpp)包含普通、边界或重复调用用例；有适合独立oracle的题还做固定种子的随机/穷举对拍。测试不是在线判题AC记录。仓库根目录可单独运行：

```sh
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0076.cpp -o /tmp/hot100-p0076
/tmp/hot100-p0076
```

复习门槛：能用自己的话复述状态含义，手推一组未见输入，指出复杂度包含哪些存储，再闭卷实现并通过测试。只记模板而说不出不变式，还需要回到手推。
