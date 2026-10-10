# 438. 找到字符串中所有字母异位词（C++17）

> 滑动窗口 · MEDIUM。前置：能读懂本题使用的容器与递归/循环状态。建议先手推，再自己写，最后看实现。

## 题意与接口边界

找s中所有长度等于p、字符出现次数与p相同的子串起点。官方输入为小写英文字母；重叠匹配也要报告。

题目与完整输入约束：[官方题页](https://leetcode.cn/problems/find-all-anagrams-in-a-string/)。本页为原创教学表达，不重复完整官方题面。

## 从朴素思路走向主解

每个窗口排序需要 O(m log m)，重复窗口高度重合。异位词等价于26维计数相等；每次右移窗口只增加一个字符、删除一个字符，不必从头重数。

## 状态与执行步骤

先统计need。遍历s时加入右端字符；若窗口超过p长，移出最左旧字符；长度到达p时比较两份26槽计数，相等就记起点。

## 手推一组自建例子

s=“baaab”、p=“aab”。前三位baa计数a2 b1，报0；窗口aaa计数a3，不报；末窗口aab再报2。s=aaaa,p=aa应报0、1、2，不能跳过重叠位置。

读到这里先暂停：不看代码，画出每次选择/指针移动后的状态；必须能解释为什么这一步安全，才算理解。

## 为什么正确

window在每次比较前恰好表示长度m的当前子串。计数相同是异位词的充要条件；每个长度m窗口恰好以某个right被检查一次，所以既不漏报也不重报起点。

## 完整 C++17 实现

源码：[可编译头文件](../../../algorithms/cpp/solutions/p0438_find_all_anagrams_in_a_string.hpp)。本地包装的namespace用于把100题同时编译；在线平台提交时只保留类及所需标准头，链表/树节点以平台提供定义为准。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0438 {
class Solution {
public:
    std::vector<int> findAnagrams(std::string s, std::string p) {
        std::vector<int> answer;
        if (p.empty() || p.size() > s.size()) return answer;
        std::array<int, 26> need{}, window{};
        for (char c : p) ++need[c - 'a'];
        for (std::size_t i = 0; i < s.size(); ++i) {
            ++window[s[i] - 'a'];
            if (i >= p.size()) --window[s[i - p.size()] - 'a'];
            if (i + 1 >= p.size() && window == need)
                answer.push_back(static_cast<int>(i + 1 - p.size()));
        }
        return answer;
    }
};
} // namespace hot100::p0438
```

## 复杂度与存储边界

固定字母表下时间 O(n+m)，其中每次比较26项是常数。算法状态空间O(26)，输出最多O(n)；当前s、p按值接口另复制O(n+m)字符，若计副本则总额外空间O(n+m)，const std::string&可避免副本。推广大字符表后，整表比较不再可以隐藏为常数。

## C++与边界陷阱

本实现对空p定义无结果；官方p非空，扩展语义需写清。c-'a'只对小写合法，不应声称支持任意字节。删除时下标i-p.size()只在i>=p.size()分支里计算，防无符号下溢。

## 面试追问：先说变化，再改算法

如何降为每步O(1)比较而不限26？维护各字符差值和非零槽数，但每次增减前后都需更新非零计数。要找包含所有p字符但长度不固定，请转最小覆盖窗口。

## 自测与迁移练习

练习1：实现差值+不相等槽数版本，与此简单版对拍。练习2：输入推广到所有字节，提示用256槽、unsigned char索引，并重新定义空p约定。

本题[可运行测试](../../../algorithms/cpp/tests/test_p0438.cpp)包含普通、边界或重复调用用例；有适合独立oracle的题还做固定种子的随机/穷举对拍。测试不是在线判题AC记录。仓库根目录可单独运行：

```sh
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0438.cpp -o /tmp/hot100-p0438
/tmp/hot100-p0438
```

复习门槛：能用自己的话复述状态含义，手推一组未见输入，指出复杂度包含哪些存储，再闭卷实现并通过测试。只记模板而说不出不变式，还需要回到手推。
