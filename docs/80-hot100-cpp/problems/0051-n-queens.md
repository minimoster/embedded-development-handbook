# 51. N 皇后（C++17）

> 回溯 · HARD。前置：能读懂本题使用的容器与递归/循环状态。建议先手推，再自己写，最后看实现。

## 题意与接口边界

在n×n棋盘放n个皇后，任意两皇后不能同行、同列或同对角线，输出所有棋盘。官方n≥1；本地n≤0返回空。

题目与完整输入约束：[官方题页](https://leetcode.cn/problems/n-queens/)。本页为原创教学表达，不重复完整官方题面。

## 从朴素思路走向主解

每行放一个皇后先满足同行限制。检查已放皇后可扫描O(n)，再观察两类对角线分别由row-col和row+col唯一标识；三个占用表让冲突判断降到O(1)。

## 状态与执行步骤

按row从0向下。枚举列，检查columns[col]、diagonalA[row-col+n-1]、diagonalB[row+col]；无冲突则标记、放Q，递归下一行；回来后撤销棋盘与三张标记。到row==n复制棋盘。

## 手推一组自建例子

n=4的一条解路径列序列[1,3,0,2]。放(0,1)后，第二行不能选列1或对角列0/2，选择3；第三行选0；第四行选2。棋盘为.Q..、...Q、Q...、..Q.。其镜像是另一解。

读到这里先暂停：不看代码，画出每次选择/指针移动后的状态；必须能解释为什么这一步安全，才算理解。

## 为什么正确

入口前row行各一皇后且两两无冲突。新位置通过三表检查就与全部旧皇后不同行列/斜线，故保持合法。每个合法棋盘有唯一逐行列序列，搜索逐行穷尽所有可选列，所以输出完整无重复。

## 完整 C++17 实现

源码：[可编译头文件](../../../algorithms/cpp/solutions/p0051_n_queens.hpp)。本地包装的namespace用于把100题同时编译；在线平台提交时只保留类及所需标准头，链表/树节点以平台提供定义为准。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0051 {
class Solution {
public:
    std::vector<std::vector<std::string>> solveNQueens(int n) {
        std::vector<std::vector<std::string>> answer;
        if (n <= 0) return answer;
        std::vector<std::string> board(n, std::string(n, '.'));
        std::vector<bool> columns(n, false), diagonalA(2*n-1, false), diagonalB(2*n-1, false);
        std::function<void(int)> visit = [&](int row) {
            if (row == n) { answer.push_back(board); return; }
            for (int col = 0; col < n; ++col) {
                int a = row - col + n - 1, b = row + col;
                if (columns[col] || diagonalA[a] || diagonalB[b]) continue;
                columns[col] = diagonalA[a] = diagonalB[b] = true; board[row][col] = 'Q';
                visit(row + 1);
                board[row][col] = '.'; columns[col] = diagonalA[a] = diagonalB[b] = false;
            }
        };
        visit(0); return answer;
    }
};
} // namespace hot100::p0051
```

## 复杂度与存储边界

以逐行列选择的松上界看，搜索节点不超过O(n!)量级且每节点扫描n列，粗界O(n·n!+Q·n²)，Q是解数；复制棋盘输出O(Qn²)。辅助棋盘O(n²)、标记/栈O(n)。不要只写O(n!)并遗漏扫描/输出。

## C++与边界陷阱

row-col可能为负，要加n-1映射到0..2n-2。撤销三表与棋盘必须都做，遗漏会过度剪枝。只判断列冲突会输出对角线攻击的无效棋盘。全局成员结果需重置，本实现局部化。

## 面试追问：先说变化，再改算法

位掩码可快速取可用列并减少数组访问，但移位宽度和补码处理需清楚，仍受指数搜索限制。若只求数量可不存棋盘；对称性剪枝要谨慎补回镜像计数。

## 自测与迁移练习

练习1：只求解数，提示省掉board复制与存储但仍维护列/斜线。练习2：实现uint64_t位掩码版本，先限定n<64，说明为何直接1ULL<<64非法，且不要承诺可在大n下快速穷举。

本题[可运行测试](../../../algorithms/cpp/tests/test_p0051.cpp)包含普通、边界或重复调用用例；有适合独立oracle的题还做固定种子的随机/穷举对拍。测试不是在线判题AC记录。仓库根目录可单独运行：

```sh
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0051.cpp -o /tmp/hot100-p0051
/tmp/hot100-p0051
```

复习门槛：能用自己的话复述状态含义，手推一组未见输入，指出复杂度包含哪些存储，再闭卷实现并通过测试。只记模板而说不出不变式，还需要回到手推。
