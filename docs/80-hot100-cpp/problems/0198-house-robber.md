# 198. 打家劫舍：C++17 推导与实现

> 分类：动态规划 · 难度：MEDIUM · 官方 Hot 100 顺序：83/100

## 题意与调用约定

房屋沿直线排列，不能选择相邻两间，求能拿到的最大总金额。金额非负；可以不选任何房屋，空数组扩展返回0。

题面与最新约束见 [力扣中文官方题目](https://leetcode.cn/problems/house-robber/) / [LeetCode 官方英文题目](https://leetcode.com/problems/house-robber/)。下文给出独立编写的推导、示例与实现；测试是本地验证，没有声称在线评测已通过。

## 从直觉到优化

对每间房屋枚举选或不选有指数分支。考虑前i间的最优解：最后一间不选，继承前i-1间；最后一间选，前一间必须不选，收益为前i-2间最优加当前金额。这两个分支覆盖所有合法方案，只需保留两项历史。

## 不变量与正确性

处理当前房之前，twoBack 是不包含前一房的前缀最优，oneBack 是包含到前一房的前缀最优。next=max(oneBack,twoBack+cash) 分别对应不选/选当前房，两者互斥且完备。滚动后 oneBack 就是新前缀最优。

## 算法步骤

1. 前0间与负长度空前缀收益都设为0。
2. 对每房求不选收益 oneBack 与选择收益 twoBack+cash。
3. 取最大值到 next，再按先旧后新的顺序移动变量。
4. 扫描结束返回 oneBack。

## 手工追踪

[4,1,2,9,3] 的前缀最优依次4、4、6、13、13。处理9时可从前两间最优4加9得到13；末尾3虽为正，也不能与9同时选，最佳仍为房0与房3，共13。

## 完整 C++17 实现

以下内容来自本课 [可编译头文件](../../../algorithms/cpp/solutions/p0198_house_robber.hpp)，namespace用于让100题可同时进入同一工程。在线提交时一般只保留评测要求的类声明，并按平台要求处理标准库命名；本地辅助头文件负责统一导入标准库。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0198 {
class Solution {
public:
    int rob(std::vector<int>& nums) {
        long long twoBack = 0, oneBack = 0;
        for (int cash : nums) {
            long long next = std::max(oneBack, twoBack + cash);
            twoBack = oneBack; oneBack = next;
        }
        return static_cast<int>(oneBack);
    }
};
} // namespace hot100::p0198
```

## 复杂度核算

时间 O(n)，额外空间 O(1)。若需恢复选择下标，通常保存 O(n) 决策或DP数组；当前接口只返回金额。

## C++ 常见陷阱

- 不能简单贪心选择当前较大房屋；它可能阻断两侧总和更大的方案。
- 变量更新前必须保存 next，twoBack 与 oneBack 各自代表不同前缀。
- 累计使用 long long；返回 int 依赖官方约束。若金额可为负且要求至少选一间，基础状态必须重设。

## 测试与运行

[完整测试源文件](../../../algorithms/cpp/tests/test_p0198.cpp)：700组固定种子小数组枚举所有不相邻选择掩码，核对最优金额；含全零、单项、空数组。

从仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0198.cpp -o /tmp/hot100-cpp/test_p0198
/tmp/hot100-cpp/test_p0198
# 环境支持时增加运行期检查：
g++ -std=c++17 -Wall -Wextra -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0198.cpp -o /tmp/hot100-cpp/test_p0198_san
ASAN_OPTIONS=detect_leaks=0 /tmp/hot100-cpp/test_p0198_san
```

本次环境的LeakSanitizer受ptrace限制，因此命令显式关闭泄漏检测；AddressSanitizer与UndefinedBehaviorSanitizer仍启用。支持泄漏检测的本地环境可移除该环境变量。断言通过说明覆盖输入上的本地结果正确，不能替代证明、完整平台约束核对或在线评测。

## 面试追问

- 房屋首尾相邻成环时如何拆成两次线性问题？
- 若房屋是树节点，选择约束如何形成选/不选两状态？

## 练习与提示

- 练习1：返回被选房屋的下标。提示：保存每个前缀最优，回溯判断是否来自前i-2。
- 练习2：允许最多选择k间。提示：给DP增加已选择数量维度。
