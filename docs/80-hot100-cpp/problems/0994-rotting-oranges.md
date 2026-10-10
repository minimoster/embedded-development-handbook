# 994. 腐烂的橘子 · C++17

> 学习位置：图论 · 第 2 课。难度：MEDIUM。这是一份独立编写的学习课，不是在线提交通过声明。

[官方题目](https://leetcode.cn/problems/rotting-oranges/) · 本课用简短契约与独立推演讲解，完整输入约束以官方题页为准。

## 1. 先说清输入、输出与修改边界

矩形网格中0为空、1为新鲜橘子、2为腐烂橘子。每分钟所有腐烂橘子同时感染四邻的新鲜橘子，求全部腐烂所需最少分钟；无法全部感染返回-1。输入会被更新为最终腐烂状态。

## 2. 从直觉到可用方案

从每个腐烂源分别搜索再取最早时间，会重复遍历网格。逐分钟扫描整个网格也能模拟，但最坏要执行很多轮整表扫描。

多源BFS把所有初始腐烂格在第0层同时入队，队列的一层对应一分钟的感染前沿。每个新鲜格第一次被某源到达时，距离已经最短；无需给每个源各建距离表。另记fresh数量，变成0时即可结束。

## 3. 为什么正确：抓住不变量

第t轮开始队列包含第t分钟已腐烂、尚未扩散的格子。四邻边的传播耗时均为1，BFS保证第一次到达新鲜格的时间最短。入队时改为2并减少fresh，使每格只被感染计数一次。若队列空仍有fresh，则剩余格与所有源不连通；若fresh归零，最后传播层数就是所需最短分钟。

## 4. 动手过程

1. 扫描网格：把所有2入队，统计1为fresh。
2. fresh=0直接得到0分钟，即使没有腐烂源也成立。
3. fresh>0且队列非空时，保存本层队列长度，恰好弹出这些格。
4. 邻接新鲜格立即改2、fresh--、入队；本层处理完minutes++。
5. 返回fresh==0?minutes:-1。

## 5. 原创例子逐步追踪

网格两行为“21112”“01110”。第0分钟源在(0,0)与(0,4)。第1分钟感染(0,1)、(0,3)；第2分钟感染(0,2)、(1,1)、(1,3)；第3分钟感染(1,2)，fresh归零，答案3。同一格(0,2)可能接近两个源，入队前标记避免被感染两次。

## 6. 时间、栈与输出空间分别算

时间O(RC)，每格入队最多一次且检查4邻。队列最坏O(RC)辅助空间，无递归栈；只存层长与计数，无独立时间矩阵。返回值O(1)。即使多个源，也不是“源数量×网格大小”。

## 7. C++17实现与所有权

- 必须先固定layerSize，不能把本轮新入队格也在同一分钟扩散。
- 入队时改2；若弹出时才标记，多个源会重复减少fresh。
- 没有新鲜橘子答案0；有新鲜但无源答案-1。
- 本实现不使用固定尺寸辅助数组，网格宽度超过10仍适用；矩形与int可容纳尺寸是接口假设。
- 算法修改输入，若调用者还需要原网格，应先复制。

下面是本仓库完整的头文件实现；support.hpp只提供标准库包含与题目节点声明，算法自身不依赖测试工具。在线评测通常已提供节点类型，提交时保留对应类并使用评测环境要求的作用域。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0994 {
class Solution {
public:
    int orangesRotting(std::vector<std::vector<int>>& grid) {
        if (grid.empty() || grid.front().empty()) return 0;
        int rows = static_cast<int>(grid.size());
        int cols = static_cast<int>(grid.front().size());
        std::queue<std::pair<int,int>> frontier;
        int fresh = 0;
        for (int r = 0; r < rows; ++r) for (int c = 0; c < cols; ++c) {
            if (grid[r][c] == 2) frontier.emplace(r, c);
            else if (grid[r][c] == 1) ++fresh;
        }
        constexpr int dr[4] = {-1, 1, 0, 0};
        constexpr int dc[4] = {0, 0, -1, 1};
        int minutes = 0;
        while (fresh > 0 && !frontier.empty()) {
            std::size_t layerSize = frontier.size();
            for (std::size_t i = 0; i < layerSize; ++i) {
                auto [r, c] = frontier.front();
                frontier.pop();
                for (int d = 0; d < 4; ++d) {
                    int nr = r + dr[d], nc = c + dc[d];
                    if (nr < 0 || nr >= rows || nc < 0 || nc >= cols || grid[nr][nc] != 1) continue;
                    grid[nr][nc] = 2;
                    --fresh;
                    frontier.emplace(nr, nc);
                }
            }
            ++minutes;
        }
        return fresh == 0 ? minutes : -1;
    }
};
} // namespace hot100::p0994
```

[查看可编译源文件](../../../algorithms/cpp/solutions/p0994_rotting_oranges.hpp) · [查看完整测试](../../../algorithms/cpp/tests/test_p0994.cpp)

## 实现复用与边界检查

固定10×10矩阵只适用于对应尺寸约束，扩展网格应按实际尺寸分配。此实现用分层 BFS 直接统计分钟，并测试全空、全烂与不可达输入。

## 9. 本地验证

400个随机网格与同步整表模拟参照比较；无新鲜、隔离、双源汇合，以及25列的尺寸扩展场景。 测试采用固定种子，可复现每轮输入。断言针对返回值，也针对应保持的地址、连接或输入状态。内存由arena或对应destroy工具明确回收。本地测试通过仅说明这些可复现用例通过，不等于完整在线评测结论。

从仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp
c++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0994.cpp -o /tmp/hot100-cpp/test_p0994
/tmp/hot100-cpp/test_p0994
# 编译器支持时，再检查地址、未定义行为与泄漏
c++ -std=c++17 -Wall -Wextra -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0994.cpp -o /tmp/hot100-cpp/test_p0994_san
ASAN_OPTIONS=detect_leaks=1 /tmp/hot100-cpp/test_p0994_san
# 若执行环境在ptrace下明确拒绝LeakSanitizer，可仅运行地址/UB检查：
# ASAN_OPTIONS=detect_leaks=0 /tmp/hot100-cpp/test_p0994_san
```

## 10. 面试延伸

- 若每条边耗时不同，BFS为何不再保证最短时间？应考虑Dijkstra，0/1权重可用0-1BFS。
- 如何输出每个橘子的腐烂时间？为网格按实际大小建立时间矩阵，首次入队记录上一格时间+1。
- 若感染必须等待两个邻居同时腐烂，这还是普通最短路吗？传播规则需要额外状态。

## 11. 两道练习与提示

1. 写同步整表模拟，与多源BFS做随机差分。提示：每分钟先收集将感染的格子，收集完成后才统一改写。
2. 返回最后一批被感染的坐标集合。提示：保存每层新增列表，仅保留最后非空层。
