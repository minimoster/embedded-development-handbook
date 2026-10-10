# 695. 岛屿的最大面积（C++17）

> 额外公开练习，不计入官方100题。前置：能读懂本题使用的容器与递归/循环状态。建议先手推，再自己写，最后看实现。

## 题意与接口边界

求0/1矩形网格中四方向连通的陆地块最大格数。此题为额外公开练习LC695，不计入当前官方Hot100的100题；本实现会把访问陆地改为0。

题目与完整输入约束：[官方题页](https://leetcode.cn/problems/max-area-of-island/)。本页为原创教学表达，不重复完整官方题面。

## 从朴素思路走向主解

题200只需计每个连通分量一次，此题把每次遍历的格数累积成面积，再取最大。用显式栈替代递归DFS，便于看到待处理格队列，也避免蛇形岛屿产生线性递归深度。

## 状态与执行步骤

扫描每个格，遇尚未访问陆地就新建pending并立刻标记；反复弹出一格使area++；四邻格合法且为陆地时先标记，再入栈；栈空后一整座岛已处理，更新best。

## 手推一组自建例子

[[1,1,0],[0,1,0],[1,0,1]]：从(0,0)的栈遍历访问(0,1)、(1,1)，面积3；下方两个单格仅对角相邻，各自面积1，所以答案3。第二次调用同一已改写网格会返回0，这是接口修改输入的明确结果。

读到这里先暂停：不看代码，画出每次选择/指针移动后的状态；必须能解释为什么这一步安全，才算理解。

## 为什么正确

入栈前标记保证同一格不会被多个邻格重复加入。四方向扩展不越过水，所以一次搜索只到达同一个陆地分量；连通性保证其全部格最终都由某条路径加入。每格恰计一次，其计数就是面积。

## 完整 C++17 实现

源码：[可编译头文件](../../../algorithms/cpp/supplementary/p0695_max_area_of_island.hpp)。本地包装的namespace用于把100题同时编译；在线平台提交时只保留类及所需标准头，链表/树节点以平台提供定义为准。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0695 {
class Solution {
public:
    int maxAreaOfIsland(std::vector<std::vector<int>>& grid) {
        if (grid.empty() || grid[0].empty()) return 0;
        const int rows = static_cast<int>(grid.size()), cols = static_cast<int>(grid[0].size());
        const int dr[]{1,-1,0,0}, dc[]{0,0,1,-1};
        int best = 0;
        for (int r = 0; r < rows; ++r) for (int c = 0; c < cols; ++c) {
            if (grid[r][c] != 1) continue;
            int area = 0;
            std::vector<std::pair<int,int>> pending{{r,c}}; grid[r][c] = 0;
            while (!pending.empty()) {
                auto [x,y] = pending.back(); pending.pop_back(); ++area;
                for (int d = 0; d < 4; ++d) {
                    int xx = x + dr[d], yy = y + dc[d];
                    if (xx >= 0 && xx < rows && yy >= 0 && yy < cols && grid[xx][yy] == 1) {
                        grid[xx][yy] = 0; pending.emplace_back(xx,yy);
                    }
                }
            }
            best = std::max(best, area);
        }
        return best;
    }
};
} // namespace hot100::p0695
```

## 复杂度与存储边界

时间O(mn)，显式栈最坏O(mn)空间；输入网格复用访问标记，不另建visited。相较递归版本，显式栈并非常数空间，只是栈容量与生命周期更可控。

## C++与边界陷阱

只认可上下左右，不能把对角相邻也算连通。标记若延迟到弹栈，可能重复入栈并错误计面积。若调用方需要保留原图，应复制网格或使用visited，并如实增加内存预算。

## 面试追问：先说变化，再改算法

若要返回最大岛的坐标集合，可在每次分量遍历中保存列表或记录编号再回扫。要处理陆地不断增加，可用并查集维护连通分量大小；删除陆地则不容易直接在线维护。

## 自测与迁移练习

练习1：返回每座岛的面积并排序，提示遍历主体不变，只改变输出聚合。练习2：估算1024×1024网格的最坏显式栈字节，提示pair<int,int>通常8字节但实际vector容量与实现需测，不能只算递归层数。

本题[可运行测试](../../../algorithms/cpp/tests/test_p0695.cpp)包含普通、边界或重复调用用例；有适合独立oracle的题还做固定种子的随机/穷举对拍。测试不是在线判题AC记录。仓库根目录可单独运行：

```sh
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0695.cpp -o /tmp/hot100-p0695
/tmp/hot100-p0695
```

复习门槛：能用自己的话复述状态含义，手推一组未见输入，指出复杂度包含哪些存储，再闭卷实现并通过测试。只记模板而说不出不变式，还需要回到手推。
