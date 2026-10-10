# 55. 跳跃游戏：C++17 推导与实现

> 分类：贪心算法 · 难度：MEDIUM · 官方 Hot 100 顺序：78/100

## 题意与调用约定

从下标0出发，nums[i] 表示该点最多向右跳多少格，可选择更短跳跃；判断能否到最后位置。元素非负；单元素无需跳跃，空数组扩展定义为不可达。

题面与最新约束见 [力扣中文官方题目](https://leetcode.cn/problems/jump-game/) / [LeetCode 官方英文题目](https://leetcode.com/problems/jump-game/)。下文给出独立编写的推导、示例与实现；测试是本地验证，没有声称在线评测已通过。

## 从直觉到优化

逐点列出所有后继，动态规划可能 O(n²)。关键是允许跳任意不超过上限的距离，所以可达位置不是零散集合，而是连续前缀。只保存这个前缀的最远端点就够了。遇到第一个超出端点的下标，后面的点也无法被访问。

## 不变量与正确性

处理到i之前，从已处理可达点能到的范围是 [0,farthest]。若 i<=farthest，则当前点确实可达，它的跳跃区间与旧前缀相交，从而把端点扩到 max(farthest,i+nums[i])。若 i>farthest，任何已可达点都跨不过缺口，目标不可达。

## 算法步骤

1. farthest 初始化0。
2. 扫描到i时先检查它是否可达；不可达立即失败。
3. 只有可达点才可以贡献新的最远距离。
4. 端点达到最后下标时立即成功。

## 手工追踪

[2,0,3,0,0,0]：从0把可达端点扩到2；位置1虽然是0，但仍可达；到2可把端点扩到5，成功。反例[1,0,2]在i=2时端点仍1，不能使用 nums[2]=2 自救。

## 完整 C++17 实现

以下内容来自本课 [可编译头文件](../../../algorithms/cpp/solutions/p0055_jump_game.hpp)，namespace用于让100题可同时进入同一工程。在线提交时一般只保留评测要求的类声明，并按平台要求处理标准库命名；本地辅助头文件负责统一导入标准库。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0055 {
class Solution {
public:
    bool canJump(std::vector<int>& nums) {
        if (nums.empty()) return false;
        long long farthest = 0;
        for (int i = 0; i < static_cast<int>(nums.size()); ++i) {
            if (i > farthest) return false;
            farthest = std::max(farthest, static_cast<long long>(i) + nums[i]);
            if (farthest >= static_cast<long long>(nums.size()) - 1) return true;
        }
        return false;
    }
};
} // namespace hot100::p0055
```

## 复杂度核算

时间 O(n)，额外空间 O(1)。不显式创建图或可达数组。

## C++ 常见陷阱

- i+nums[i] 用 long long，避免大跳跃值的 int 溢出。
- 不能先更新端点再检查是否可达，否则会让不可达节点贡献跳跃。
- 这项贪心依赖“可选任意较短距离”，如果只能恰好跳 nums[i] 格，前缀性质不成立。

## 测试与运行

[完整测试源文件](../../../algorithms/cpp/tests/test_p0055.cpp)：穷举长度1到8、跳长0到3的所有数组，与显式可达状态DP对拍；覆盖中途断路、零步单项及空扩展。

从仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0055.cpp -o /tmp/hot100-cpp/test_p0055
/tmp/hot100-cpp/test_p0055
# 环境支持时增加运行期检查：
g++ -std=c++17 -Wall -Wextra -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0055.cpp -o /tmp/hot100-cpp/test_p0055_san
ASAN_OPTIONS=detect_leaks=0 /tmp/hot100-cpp/test_p0055_san
```

本次环境的LeakSanitizer受ptrace限制，因此命令显式关闭泄漏检测；AddressSanitizer与UndefinedBehaviorSanitizer仍启用。支持泄漏检测的本地环境可移除该环境变量。断言通过说明覆盖输入上的本地结果正确，不能替代证明、完整平台约束核对或在线评测。

## 面试追问

- 如何在 O(n) 时间同时找出一条合法跳跃路径？
- 允许向左跳、障碍或跳跃费用时，为什么需要改变状态模型？

## 练习与提示

- 练习1：实现显式可达数组作为对拍基线。提示：只从已标记可达的下标扩散。
- 练习2：找“只能恰好跳指定步数”与本题答案不同的输入。提示：利用越过终点而不能落下的情形。
