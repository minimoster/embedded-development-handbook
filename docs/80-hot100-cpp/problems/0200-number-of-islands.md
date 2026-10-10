# 200. 岛屿数量 · C++17

> 学习位置：图论 · 第 1 课。难度：MEDIUM。这是一份独立编写的学习课，不是在线提交通过声明。

[官方题目](https://leetcode.cn/problems/number-of-islands/) · 本课用简短契约与独立推演讲解，完整输入约束以官方题页为准。

## 1. 先说清输入、输出与修改边界

矩形字符网格中，'1'表示陆地、'0'表示水。只按上下左右相邻连接，求陆地连通块数；对角接触不连通。本实现会把访问过的'1'改为'0'，因此调用后输入网格被消耗。

## 2. 从直觉到可用方案

为每一块陆地启动搜索，若没有记录访问状态，就会重复计算同一个岛。每次遇到尚未访问的'1'，它必定代表一个尚未计数的连通块：先加一，再用洪泛填充把整个块标为已访问。

递归DFS简短，但细长岛可能让调用栈深到R×C。这里使用显式vector栈，并在入栈时就把单元格改为'0'，避免同一个格子被多个邻居重复压栈。

## 3. 为什么正确：抓住不变量

外层扫描到某格时，所有已计数连通块都已被完整标记。新遇到'1'不可能属于旧块，否则它会被之前的四邻搜索访问。搜索只通过陆地四邻扩展，恰好覆盖当前块，且入栈即标记保证每格至多入栈一次。因此每次计数与一个真实岛一一对应。

## 4. 动手过程

1. 空网格或空首行返回0，取得rows、cols。
2. 行列扫描，遇到水跳过。
3. 遇到陆地计数加一，先标水，再压入栈。
4. 弹出格子，检查四个邻居的边界与陆地标记；合法陆地入栈时立即标水。
5. 当前栈清空后继续外层扫描。

## 5. 原创例子逐步追踪

网格三行为“11010”“01010”“00100”。左上(0,0)、(0,1)、(1,1)连成第一岛；(0,3)、(1,3)为第二岛；(2,2)为第三岛。虽(1,1)与(2,2)斜着接触，仍不连通。扫描到(0,0)后第一岛全部变水，随后(0,1)就不会再次计数。

## 6. 时间、栈与输出空间分别算

矩形R×C网格中，每格检查与入栈次数为常数，总时间O(RC)。显式栈最坏O(RC)，无递归栈；就地标记无需额外visited矩阵，返回值空间O(1)。若要求保留输入，复制网格还需O(RC)空间和时间。

## 7. C++17实现与所有权

- 入栈前标记，而非弹出后标记，否则环状邻接会重复入栈。
- 先检查grid.empty()，不能无条件读取grid[0]。
- rows/cols转为int是基于题目尺寸能放入int；超大容器工程接口需检查转换范围。
- 只适用于矩形网格；若每行长度不同，必须按各行边界重新定义邻接。
- 不分配节点，所有容器自动回收；输出计数int足够覆盖官方范围。

下面是本仓库完整的头文件实现；support.hpp只提供标准库包含与题目节点声明，算法自身不依赖测试工具。在线评测通常已提供节点类型，提交时保留对应类并使用评测环境要求的作用域。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0200 {
class Solution {
public:
    int numIslands(std::vector<std::vector<char>>& grid) {
        if (grid.empty() || grid.front().empty()) return 0;
        int rows = static_cast<int>(grid.size());
        int cols = static_cast<int>(grid.front().size());
        constexpr int dr[4] = {-1, 1, 0, 0};
        constexpr int dc[4] = {0, 0, -1, 1};
        int islands = 0;
        std::vector<std::pair<int,int>> pending;
        for (int r = 0; r < rows; ++r) for (int c = 0; c < cols; ++c) {
            if (grid[r][c] != '1') continue;
            ++islands;
            grid[r][c] = '0';
            pending.emplace_back(r, c);
            while (!pending.empty()) {
                auto [x, y] = pending.back();
                pending.pop_back();
                for (int d = 0; d < 4; ++d) {
                    int nx = x + dr[d], ny = y + dc[d];
                    if (nx < 0 || nx >= rows || ny < 0 || ny >= cols || grid[nx][ny] != '1') continue;
                    grid[nx][ny] = '0';
                    pending.emplace_back(nx, ny);
                }
            }
        }
        return islands;
    }
};
} // namespace hot100::p0200
```

[查看可编译源文件](../../../algorithms/cpp/solutions/p0200_number_of_islands.hpp) · [查看完整测试](../../../algorithms/cpp/tests/test_p0200.cpp)

## 实现复用与边界检查

递归 DFS 的最坏调用栈可达整个陆地块规模。并查集或搜索状态若为成员，每次入口必须重建或清空；此实现用局部显式栈与就地标记。

## 9. 本地验证

350个随机网格与独立并查集计数参照比较；对角不连通、空网格、同对象重复调用和10000格细长岛。 测试采用固定种子，可复现每轮输入。断言针对返回值，也针对应保持的地址、连接或输入状态。内存由arena或对应destroy工具明确回收。本地测试通过仅说明这些可复现用例通过，不等于完整在线评测结论。

从仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp
c++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0200.cpp -o /tmp/hot100-cpp/test_p0200
/tmp/hot100-cpp/test_p0200
# 编译器支持时，再检查地址、未定义行为与泄漏
c++ -std=c++17 -Wall -Wextra -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0200.cpp -o /tmp/hot100-cpp/test_p0200_san
ASAN_OPTIONS=detect_leaks=1 /tmp/hot100-cpp/test_p0200_san
# 若执行环境在ptrace下明确拒绝LeakSanitizer，可仅运行地址/UB检查：
# ASAN_OPTIONS=detect_leaks=0 /tmp/hot100-cpp/test_p0200_san
```

## 10. 面试延伸

- 动态逐个增加陆地并实时报告岛数，为什么并查集更合适？新块计数+1，每合并两个不同根再-1。
- 若邻接包含对角线，答案和方向表怎样变化？使用八方向，证明仍是连通块洪泛。
- 若不能修改grid，访问状态在哪里保存？

## 11. 两道练习与提示

1. 写不改输入的BFS版本，与本课结果比较。提示：独立visited矩阵，入队时标记。
2. 同次搜索统计每岛面积与周长。提示：面积按访问格数累加；每条朝水或越界的边给周长+1。
