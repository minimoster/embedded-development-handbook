# 49. 字母异位词分组（C++17）

> 哈希 · MEDIUM。前置：能读懂本题使用的容器与递归/循环状态。建议先手推，再自己写，最后看实现。

## 题意与接口边界

把字母多重集合相同的单词归为一组。组顺序和组内顺序不作为正确性要求；重复单词仍保留。

题目与完整输入约束：[官方题页](https://leetcode.cn/problems/group-anagrams/)。本页为原创教学表达，不重复完整官方题面。

## 从朴素思路走向主解

两个字符串重新排列后相同，等价于排序后的字符串相同。先选一个可靠的规范键，再把分组当成哈希聚合。这里选排序键，让字符计数的编码细节不掩盖核心思想。

## 状态与执行步骤

逐词复制生成 key；对 key 排序；若 key 首次出现，创建一个新组并记下组下标；否则追加到已有组。输入词本身不排序，结果保存原词。

## 手推一组自建例子

[pots,stop,abc,tops,空串] 的键依次为 opst、opst、abc、opst、空串。第一次 opst 建组0，后两个同键追加；abc 与空串各自形成独立组。

读到这里先暂停：不看代码，画出每次选择/指针移动后的状态；必须能解释为什么这一步安全，才算理解。

## 为什么正确

排序保留字符及其出现次数，且规范后的相同字符序列唯一。因此同键的词一定互为异位词；互为异位词一定产生同键。哈希表只负责把相同键送进同一容器。

## 完整 C++17 实现

源码：[可编译头文件](../../../algorithms/cpp/solutions/p0049_group_anagrams.hpp)。本地包装的namespace用于把100题同时编译；在线平台提交时只保留类及所需标准头，链表/树节点以平台提供定义为准。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0049 {
class Solution {
public:
    std::vector<std::vector<std::string>> groupAnagrams(std::vector<std::string>& words) {
        std::unordered_map<std::string, std::size_t> group;
        std::vector<std::vector<std::string>> answer;
        for (const auto& word : words) {
            std::string key = word;
            std::sort(key.begin(), key.end());
            auto it = group.find(key);
            if (it == group.end()) {
                std::size_t index = answer.size();
                group.emplace(key, index);
                answer.push_back({word});
            } else answer[it->second].push_back(word);
        }
        return answer;
    }
};
} // namespace hot100::p0049
```

## 复杂度与存储边界

n个词、最长长度L，时间 O(n L log L) 加哈希与复制开销，额外键/输出空间 O(nL)。把结果存储算进去不能写 O(n) 字节；哈希复杂度按均摊讨论。

## C++与边界陷阱

用 const auto& 避免不必要的原词复制，但 key 必须复制。不能拿整个 vector<int> 直接做 unordered_map 键而不提供哈希。若用计数拼接键，要分隔数字，避免[1,11]与[11,1]之类歧义。

## 面试追问：先说变化，再改算法

仅小写英文字母时可用26维计数键，将规范化降到 O(L)，但需要自定义哈希或无歧义编码。Unicode输入还要先定义字素、大小写与规范化规则。

## 自测与迁移练习

练习1：实现26维array键并为array写哈希，提示还需正确的相等比较。练习2：比较每组的出现顺序与字典序，提示排序输出是额外要求，不能凭 unordered_map 迭代顺序保证稳定。

本题[可运行测试](../../../algorithms/cpp/tests/test_p0049.cpp)包含普通、边界或重复调用用例；有适合独立oracle的题还做固定种子的随机/穷举对拍。测试不是在线判题AC记录。仓库根目录可单独运行：

```sh
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0049.cpp -o /tmp/hot100-p0049
/tmp/hot100-p0049
```

复习门槛：能用自己的话复述状态含义，手推一组未见输入，指出复杂度包含哪些存储，再闭卷实现并通过测试。只记模板而说不出不变式，还需要回到手推。
