# 79. 单词搜索（C++17）

> 回溯 · MEDIUM。前置：能读懂本题使用的容器与递归/循环状态。建议先手推，再自己写，最后看实现。

## 题意与接口边界

在矩形字符网格中沿上下左右相邻格拼出word，每次路径不能重复使用一个格。输入保持不变；本地空word定义为存在。

题目与完整输入约束：[官方题页](https://leetcode.cn/problems/word-search/)。本页为原创教学表达，不重复完整官方题面。

## 从朴素思路走向主解

只找到各字母出现并不足够，还必须连成路径。每个格可作为起点，DFS依次匹配word[index]。访问状态必须是当前路径局部占用，而不是永久全图标记，否则不同分支会相互干扰。

## 状态与执行步骤

先处理空词和空板；若词长超过格数直接拒绝。DFS校验边界、未用状态、字符；最后一个字母匹配就成功。否则标记当前格，试四邻格，再无条件恢复标记后返回成功状态。

## 手推一组自建例子

板[[A,B],[D,C]]可从A→B→C→D拼ABCD，四格各用一次。ABA不能从A走B再回A，因为A在当前路径已标记。先查ABCD再查ABA应仍返回false，不受先前成功污染。

读到这里先暂停：不看代码，画出每次选择/指针移动后的状态；必须能解释为什么这一步安全，才算理解。

## 为什么正确

递归入口已匹配前index个字母且used恰标记此前路径格。校验后当前格合法且不重复；扩展所有四邻格穷尽合法下一步。恢复标记使兄弟路线独立，成功链完整匹配每个字母，故存在性判定正确。

## 完整 C++17 实现

源码：[可编译头文件](../../../algorithms/cpp/solutions/p0079_word_search.hpp)。本地包装的namespace用于把100题同时编译；在线平台提交时只保留类及所需标准头，链表/树节点以平台提供定义为准。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0079 {
class Solution {
public:
    bool exist(std::vector<std::vector<char>>& board, std::string word) {
        if (word.empty()) return true;
        if (board.empty() || board[0].empty()) return false;
        int rows = static_cast<int>(board.size()), cols = static_cast<int>(board[0].size());
        if (word.size() > static_cast<std::size_t>(rows) * cols) return false;
        std::vector<std::vector<bool>> used(rows, std::vector<bool>(cols, false));
        const int dr[4]{1,-1,0,0}, dc[4]{0,0,1,-1};
        std::function<bool(int,int,std::size_t)> visit = [&](int r, int c, std::size_t index) {
            if (r < 0 || r >= rows || c < 0 || c >= cols || used[r][c] || board[r][c] != word[index]) return false;
            if (index + 1 == word.size()) return true;
            used[r][c] = true;
            bool found = false;
            for (int d = 0; d < 4 && !found; ++d) found = visit(r + dr[d], c + dc[d], index + 1);
            used[r][c] = false;
            return found;
        };
        for (int r = 0; r < rows; ++r) for (int c = 0; c < cols; ++c)
            if (visit(r, c, 0)) return true;
        return false;
    }
};
} // namespace hot100::p0079
```

## 复杂度与存储边界

宽松上界O(mn·4^L)，考虑除首次不能回到上一格可写O(mn·3^L)的常用界；辅助used O(mn)、递归O(L)，L≤mn。输入未改写以换取清楚的生命周期。

## C++与边界陷阱

成功提前返回也要先恢复当前标记。本实现将found存局部后统一回退。若使用成员res且入口未重置，同一对象先成功后查别词会残留true；本课测试专门覆盖连续调用。

## 面试追问：先说变化，再改算法

可先统计字母频次做必要条件拒绝，还可从更少见的词端开始搜索（反转词），减少分支但不改变最坏阶。多单词搜索适合Trie，不能为每个词重复建一遍同样状态。

## 自测与迁移练习

练习1：用board临时哨兵标记替代used并恢复，提示哨兵不能与合法字符冲突。练习2：写独立穷举路径oracle对拍2×3板，提示oracle用位mask记录路径格，保证不会复用主解visited代码。

本题[可运行测试](../../../algorithms/cpp/tests/test_p0079.cpp)包含普通、边界或重复调用用例；有适合独立oracle的题还做固定种子的随机/穷举对拍。测试不是在线判题AC记录。仓库根目录可单独运行：

```sh
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0079.cpp -o /tmp/hot100-p0079
/tmp/hot100-p0079
```

复习门槛：能用自己的话复述状态含义，手推一组未见输入，指出复杂度包含哪些存储，再闭卷实现并通过测试。只记模板而说不出不变式，还需要回到手推。
