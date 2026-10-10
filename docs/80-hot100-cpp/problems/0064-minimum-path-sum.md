# 64. 最小路径和：C++17 推导与实现

> 分类：多维动态规划 · 难度：MEDIUM · 官方 Hot 100 顺序：92/100

## 题意与调用约定

规则非负整数网格，只能向右或向下，从左上走到右下，最小化沿途格值总和，起点和终点都计入。实现不修改原网格，空网格扩展返回0。

题面与最新约束见 [力扣中文官方题目](https://leetcode.cn/problems/minimum-path-sum/) / [LeetCode 官方英文题目](https://leetcode.com/problems/minimum-path-sum/)。下文给出独立编写的推导、示例与实现；测试是本地验证，没有声称在线评测已通过。

## 从直觉到优化

每条路线逐个计算总和会重复探索。任意到达一个格子的路线，最后一步来自上方或左方；只保留两种来源中的较小成本即可。二维状态可压成一行，更新顺序仍是从左到右，但第一行/第一列要避免把不存在的来源当成0。

## 不变量与正确性

更新 col 时，cost[col] 是上方格子的最优成本，cost[col-1] 是本行左方最优成本。不存在的来源用 infinity 表示。初始 cost[0]=0 相当于为起点添加一个唯一零成本虚拟来源，其余位置不可达；逐格取min加格值得到所有路径的最小值。

## 算法步骤

1. 初始化整行不可达，只给第0列虚拟起点成本0。
2. 逐行逐列扫描，左边不存在时使用 infinity。
3. cost[col]=min(旧上方,新左方)+当前值。
4. 末项即目标成本，保留原网格内容。

## 手工追踪

网格 [[2,8,1],[1,3,2],[6,1,4]]。第一行成本[2,10,11]；第二行[3,6,8]；第三行[9,7,11]。目标11可走2→1→3→1→4。每一行的数是到该格的全局成本，不是格值本身。

## 完整 C++17 实现

以下内容来自本课 [可编译头文件](../../../algorithms/cpp/solutions/p0064_minimum_path_sum.hpp)，namespace用于让100题可同时进入同一工程。在线提交时一般只保留评测要求的类声明，并按平台要求处理标准库命名；本地辅助头文件负责统一导入标准库。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0064 {
class Solution {
public:
    int minPathSum(std::vector<std::vector<int>>& grid) {
        if (grid.empty() || grid.front().empty()) return 0;
        std::size_t cols = grid.front().size();
        const long long infinity = std::numeric_limits<long long>::max() / 4;
        std::vector<long long> cost(cols, infinity); cost[0] = 0;
        for (const auto& row : grid) {
            for (std::size_t col = 0; col < cols; ++col) {
                long long left = col ? cost[col-1] : infinity;
                cost[col] = std::min(cost[col], left) + row[col];
            }
        }
        return static_cast<int>(cost.back());
    }
};
} // namespace hot100::p0064
```

## 复杂度核算

时间 O(mn)，辅助空间 O(n)（n为列数），返回标量。若原地覆盖输入，可省到 O(1) 辅助空间但会改变网格；本课选择保留输入。

## C++ 常见陷阱

- 用0代替不存在来源会让第一行从空中免费开始。
- infinity 取 LLONG_MAX/4，为加法留余量；官方格值和路径长度范围仍是安全前提。
- 必须是规则矩形，行长不一致会索引越界；若泛化接口，应先验证每行长度。

## 测试与运行

[完整测试源文件](../../../algorithms/cpp/tests/test_p0064.cpp)：800组固定种子至多5×5网格与路线递归枚举对拍；含单格、单行、空网格，并保证实现不修改输入。

从仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0064.cpp -o /tmp/hot100-cpp/test_p0064
/tmp/hot100-cpp/test_p0064
# 环境支持时增加运行期检查：
g++ -std=c++17 -Wall -Wextra -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0064.cpp -o /tmp/hot100-cpp/test_p0064_san
ASAN_OPTIONS=detect_leaks=0 /tmp/hot100-cpp/test_p0064_san
```

本次环境的LeakSanitizer受ptrace限制，因此命令显式关闭泄漏检测；AddressSanitizer与UndefinedBehaviorSanitizer仍启用。支持泄漏检测的本地环境可移除该环境变量。断言通过说明覆盖输入上的本地结果正确，不能替代证明、完整平台约束核对或在线评测。

## 面试追问

- 网格可含负数时，单向无环DP仍成立吗？
- 若能四方向移动且出现负环，为什么这个局部递推不再成立？

## 练习与提示

- 练习1：记录最短路径。提示：另存每格来自上方还是左方的决策。
- 练习2：允许障碍格。提示：障碍把对应滚动成本改成不可达，目标不可达时别返回哨兵。
