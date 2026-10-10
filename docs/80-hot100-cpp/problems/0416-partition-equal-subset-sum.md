# 416. 分割等和子集：C++17 推导与实现

> 分类：动态规划 · 难度：MEDIUM · 官方 Hot 100 顺序：89/100

## 题意与调用约定

非空正整数数组，判断能否把所有元素分到两组，使两组和相同。每个位置只使用一次，重复值仍是不同可选元素。

题面与最新约束见 [力扣中文官方题目](https://leetcode.cn/problems/partition-equal-subset-sum/) / [LeetCode 官方英文题目](https://leetcode.com/problems/partition-equal-subset-sum/)。下文给出独立编写的推导、示例与实现；测试是本地验证，没有声称在线评测已通过。

## 从直觉到优化

总和为奇数立即不可能。偶数时问题变为“是否存在子集和为总和一半”。二维0/1背包可按已处理元素与目标金额建表；本课压成一维布尔表，但必须倒序更新金额，避免同一个元素在一轮内被重复选。

## 不变量与正确性

处理完前k个元素后，possible[t] 表示可从这些元素中选出和t。处理新value时，倒序保证 possible[t-value] 仍是上一轮状态；因此 possible[t] 由不选该项或选一次该项形成。找到target的子集后，其余元素恰好也是target。

## 算法步骤

1. 计算总和，奇数直接false。
2. 目标为sum/2，possible[0]=true。
3. 每个元素都从target倒序到value，合并旧可达与旧t-value可达。
4. 返回target可达性。

## 手工追踪

[3,3,4,2] 总和12目标6。第一个3使3可达；第二个3倒序处理6时发现旧3可达，令6可达，得到两组[3,3]与[4,2]。若只有一个3且正序处理，可能同轮让3再贡献到6，错误重复使用。

## 完整 C++17 实现

以下内容来自本课 [可编译头文件](../../../algorithms/cpp/solutions/p0416_partition_equal_subset_sum.hpp)，namespace用于让100题可同时进入同一工程。在线提交时一般只保留评测要求的类声明，并按平台要求处理标准库命名；本地辅助头文件负责统一导入标准库。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0416 {
class Solution {
public:
    bool canPartition(std::vector<int>& nums) {
        long long sum = std::accumulate(nums.begin(), nums.end(), 0LL);
        if (sum % 2) return false;
        int target = static_cast<int>(sum / 2);
        std::vector<unsigned char> possible(target + 1, 0); possible[0] = 1;
        for (int value : nums)
            for (int total = target; total >= value; --total)
                possible[total] = possible[total] || possible[total-value];
        return possible[target] != 0;
    }
};
} // namespace hot100::p0416
```

## 复杂度核算

设总和S、目标T=S/2，时间 O(nT)，额外空间 O(T)，这是依赖数值大小的伪多项式复杂度；累加总和 O(n)。无递归。

## C++ 常见陷阱

- std::accumulate 的初值必须0LL，最终赋给 long long 不能修复中途 int 溢出。
- target转int依赖官方总和上限；泛化到极大总和需要改数据结构或另一算法。
- 倒序的 total 使用有符号int，避免无符号 total-- 从0下溢；正整数前提让循环正确终止。

## 测试与运行

[完整测试源文件](../../../algorithms/cpp/tests/test_p0416.cpp)：1000组固定种子正整数数组，与全部子集掩码对拍；含奇数总和、单项与重复值分别可选。

从仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0416.cpp -o /tmp/hot100-cpp/test_p0416
/tmp/hot100-cpp/test_p0416
# 环境支持时增加运行期检查：
g++ -std=c++17 -Wall -Wextra -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0416.cpp -o /tmp/hot100-cpp/test_p0416_san
ASAN_OPTIONS=detect_leaks=0 /tmp/hot100-cpp/test_p0416_san
```

本次环境的LeakSanitizer受ptrace限制，因此命令显式关闭泄漏检测；AddressSanitizer与UndefinedBehaviorSanitizer仍启用。支持泄漏检测的本地环境可移除该环境变量。断言通过说明覆盖输入上的本地结果正确，不能替代证明、完整平台约束核对或在线评测。

## 面试追问

- 如何输出实际两组元素？
- n较小但数值极大时，折半枚举与数值DP的取舍是什么？

## 练习与提示

- 练习1：用二维DP验证一维倒序版。提示：每轮转移只读上一行。
- 练习2：证明正序会变完全背包并写出失败样例。提示：[1,2,5] 目标4会错误重复使用1或2。
