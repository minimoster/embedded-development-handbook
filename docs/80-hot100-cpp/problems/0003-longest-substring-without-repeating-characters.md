# 3. 无重复字符的最长子串（C++17）

> 滑动窗口 · MEDIUM。前置：能读懂本题使用的容器与递归/循环状态。建议先手推，再自己写，最后看实现。

## 题意与接口边界

找没有重复字符的最长连续子串长度。此C++版本按字节判断字符，适合官方ASCII类输入，不把UTF-8字节冒充Unicode字素。

题目与完整输入约束：[官方题页](https://leetcode.cn/problems/longest-substring-without-repeating-characters/)。本页为原创教学表达，不重复完整官方题面。

## 从朴素思路走向主解

枚举所有子串并判重会重复做事。用窗口保持唯一性；新字符若曾出现在窗口里，左边界至少要跨过那次出现。记录每个字符最近位置，可以一次跳过去，免去逐步删除。

## 状态与执行步骤

last初始化-1。每个right取字符c，left=max(left,last[c]+1)；窗口修复后更新长度，再把last[c]改为right。left始终只前进，历史出现位置在窗口左侧时不影响当前窗口。

## 手推一组自建例子

“abba”：a时left0、长1；b时长2；第二个b把left升到2；最后a的旧位置0已在窗口外，max防止left退回1，因此长度仍2。

读到这里先暂停：不看代码，画出每次选择/指针移动后的状态；必须能解释为什么这一步安全，才算理解。

## 为什么正确

right进入后只有新字符可能造成重复，其他字符原来已经唯一。跨过最近一次同字符即可消除此唯一冲突；更早同字符也随之被排除。左边界取必要的最小值，所以当前窗口是以right结尾的最长合法窗口。

## 完整 C++17 实现

源码：[可编译头文件](../../../algorithms/cpp/solutions/p0003_longest_substring_without_repeating_characters.hpp)。本地包装的namespace用于把100题同时编译；在线平台提交时只保留类及所需标准头，链表/树节点以平台提供定义为准。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0003 {
class Solution {
public:
    int lengthOfLongestSubstring(std::string s) {
        std::array<int, 256> last; last.fill(-1);
        int left = 0, best = 0;
        for (int right = 0; right < static_cast<int>(s.size()); ++right) {
            unsigned char c = static_cast<unsigned char>(s[right]);
            left = std::max(left, last[c] + 1);
            best = std::max(best, right - left + 1);
            last[c] = right;
        }
        return best;
    }
};
} // namespace hot100::p0003
```

## 复杂度与存储边界

时间 O(n)，固定256槽算法状态空间 O(1)。当前接口std::string按值传参，还会生成O(n)输入副本；若计入这份副本，额外空间为O(n)，改const std::string&可避免它。若字符集非固定，改哈希后状态空间 O(min(n,字符种类))并按均摊时间讨论。

## C++与边界陷阱

char可能有符号，直接把负char做数组下标是未定义行为；必须转unsigned char。left不能直接赋last[c]+1，否则旧字符会让窗口退回。返回长度类型int依赖本题字符串长度范围。

## 面试追问：先说变化，再改算法

要返回子串可同时记录最优起点。若要求至多K个不同字符，需要频率表与收缩循环，最近位置技巧不直接替代频率。Unicode版本需先拆码点或字素再滑动。

## 自测与迁移练习

练习1：返回最长子串本身并定平局策略，提示保存bestLeft而不是结束后猜left。练习2：改为最长至多两种字符，提示条件是种类计数，移左时频率归零才减少种类数。

本题[可运行测试](../../../algorithms/cpp/tests/test_p0003.cpp)包含普通、边界或重复调用用例；有适合独立oracle的题还做固定种子的随机/穷举对拍。测试不是在线判题AC记录。仓库根目录可单独运行：

```sh
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0003.cpp -o /tmp/hot100-p0003
/tmp/hot100-p0003
```

复习门槛：能用自己的话复述状态含义，手推一组未见输入，指出复杂度包含哪些存储，再闭卷实现并通过测试。只记模板而说不出不变式，还需要回到手推。
