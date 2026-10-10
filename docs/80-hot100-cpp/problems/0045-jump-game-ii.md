# 45. 跳跃游戏 II：C++17 推导与实现

> 分类：贪心算法 · 难度：MEDIUM · 官方 Hot 100 顺序：79/100

## 题意与调用约定

跳跃规则与第55题相同，求到末位置的最少跳跃次数。官方保证可达；本实现额外对不可达数组返回 -1，对空数组返回0。

题面与最新约束见 [力扣中文官方题目](https://leetcode.cn/problems/jump-game-ii/) / [LeetCode 官方英文题目](https://leetcode.com/problems/jump-game-ii/)。下文给出独立编写的推导、示例与实现；测试是本地验证，没有声称在线评测已通过。

## 从直觉到优化

最短路径可以做 BFS，但一个点有许多后继会产生 O(n²) 边。各层可达位置是连续区间：扫描当前层时累积下一层最远端点，到当前层末端才把跳跃数加一。这是压缩了邻接边的 BFS，而不是每次真跳到“眼前最远点”。

## 不变量与正确性

layerEnd 是用 steps 次跳跃能够覆盖的最远位置；在扫描该层时 nextEnd 记录再跳一次所能到的最远位置。只有当前层完整扫描后才能提交新边界，这保证所有更短路径已被考察，第一次覆盖终点的层数就是最少次数。

## 算法步骤

1. steps=0，当前层末端0，下一层末端0。
2. 扫描每个可达位置，更新 i+nums[i] 的最大值。
3. 到 layerEnd 时提交下一层，steps 加一。
4. 若边界不增长返回不可达；新边界覆盖终点就返回步骤数。

## 手工追踪

[3,1,1,2,0,1]：第0层只有位置0，第一跳覆盖到3，steps=1；扫描位置1、2、3，最远候选分别2、3、5。到3提交第2层，覆盖终点5，所以最少2跳，可走0→3→5。

## 完整 C++17 实现

以下内容来自本课 [可编译头文件](../../../algorithms/cpp/solutions/p0045_jump_game_ii.hpp)，namespace用于让100题可同时进入同一工程。在线提交时一般只保留评测要求的类声明，并按平台要求处理标准库命名；本地辅助头文件负责统一导入标准库。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0045 {
class Solution {
public:
    int jump(std::vector<int>& nums) {
        int n = static_cast<int>(nums.size());
        if (n <= 1) return 0;
        long long layerEnd = 0, nextEnd = 0;
        int steps = 0;
        for (int i = 0; i < n - 1; ++i) {
            if (i > nextEnd) return -1;
            nextEnd = std::max(nextEnd, static_cast<long long>(i) + nums[i]);
            if (i == layerEnd) {
                if (nextEnd == layerEnd) return -1;
                ++steps; layerEnd = nextEnd;
                if (layerEnd >= n - 1) return steps;
            }
        }
        return -1;
    }
};
} // namespace hot100::p0045
```

## 复杂度核算

时间 O(n)，额外空间 O(1)。没有显式 BFS 队列或 O(n²) 的边；返回标量。

## C++ 常见陷阱

- 循环只扫描到 n-2，终点无需再起跳；否则可能多加一次。
- 只有到当前层末端才 ++steps，不能每遇到更远候选就加。
- 边界采用 long long；不可达时不增长必须处理，否则会错误返回某个次数。

## 测试与运行

[完整测试源文件](../../../algorithms/cpp/tests/test_p0045.cpp)：穷举长度1到8、跳长0到3的所有数组，与逐边最短步数DP对拍；同时验证官方可达输入和额外不可达返回-1。

从仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0045.cpp -o /tmp/hot100-cpp/test_p0045
/tmp/hot100-cpp/test_p0045
# 环境支持时增加运行期检查：
g++ -std=c++17 -Wall -Wextra -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0045.cpp -o /tmp/hot100-cpp/test_p0045_san
ASAN_OPTIONS=detect_leaks=0 /tmp/hot100-cpp/test_p0045_san
```

本次环境的LeakSanitizer受ptrace限制，因此命令显式关闭泄漏检测；AddressSanitizer与UndefinedBehaviorSanitizer仍启用。支持泄漏检测的本地环境可移除该环境变量。断言通过说明覆盖输入上的本地结果正确，不能替代证明、完整平台约束核对或在线评测。

## 面试追问

- 怎样恢复一条最短路径而不只是步数？
- 为什么“跳到允许范围内下标最大的点”不是正确贪心？

## 练习与提示

- 练习1：构造最远落点策略失败的数组。提示：较近的落点可能有更大跳跃能力。
- 练习2：用普通 DP 对所有长度不超过8的小数组对拍。提示：不可达用 n+1 表示。
