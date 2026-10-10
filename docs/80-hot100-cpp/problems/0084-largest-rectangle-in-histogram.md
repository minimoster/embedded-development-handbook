# 84. 柱状图中最大的矩形：C++17 推导与实现

> 分类：栈 · 难度：HARD · 官方 Hot 100 顺序：73/100

## 题意与调用约定

相邻柱子宽度均为1、高度非负，求完全落在柱状图内的最大矩形面积。允许跨越多个柱子，但高度不能超过覆盖区间最低柱。

题面与最新约束见 [力扣中文官方题目](https://leetcode.cn/problems/largest-rectangle-in-histogram/) / [LeetCode 官方英文题目](https://leetcode.com/problems/largest-rectangle-in-histogram/)。下文给出独立编写的推导、示例与实现；测试是本地验证，没有声称在线评测已通过。

## 从直觉到优化

枚举所有左右端点并维护最低柱可做 O(n²)。反过来为每个柱子考虑“以它为限制高度的最大宽度”。遇到右侧第一个更矮柱时，就能结算该柱的右边界；单调非递减栈保存尚未结算的柱。末尾虚拟零高柱把未结算正高度全部清空。

## 不变量与正确性

栈内下标递增、高度非递减。弹出高度h的 top 时，当前 i 是它右侧首个严格更矮位置；弹出后的左端栈顶 left 高度不大于h，区间 (left,i) 全部至少h。存在等高柱时，这一项可能不是最宽，但最左的等高代表最终会获得完整宽度，所以不会漏最大矩形。

## 算法步骤

1. 从0扫描到n，i=n时使用虚拟高度0，不改原数组。
2. 当前高度更低时，反复弹出栈顶柱。
3. 弹出后用新栈顶或-1作左边界，宽度为 i-left-1。
4. 以 long long 计算面积并更新最大值，真实下标再入栈。

## 手工追踪

[3,1,4,4,2]：到高度1时弹出3，面积3；到下标4的高度2时先弹下标3的4，宽1面积4，再弹下标2的4，左界1、右界4，宽2面积8。虚拟0再结算2和1，最大值仍8。

## 完整 C++17 实现

以下内容来自本课 [可编译头文件](../../../algorithms/cpp/solutions/p0084_largest_rectangle_in_histogram.hpp)，namespace用于让100题可同时进入同一工程。在线提交时一般只保留评测要求的类声明，并按平台要求处理标准库命名；本地辅助头文件负责统一导入标准库。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0084 {
class Solution {
public:
    int largestRectangleArea(std::vector<int>& heights) {
        int n = static_cast<int>(heights.size());
        std::vector<int> increasing;
        long long best = 0;
        for (int i = 0; i <= n; ++i) {
            int current = i == n ? 0 : heights[i];
            while (!increasing.empty() && heights[increasing.back()] > current) {
                int top = increasing.back(); increasing.pop_back();
                int left = increasing.empty() ? -1 : increasing.back();
                long long width = i - left - 1;
                best = std::max(best, width * heights[top]);
            }
            if (i < n) increasing.push_back(i);
        }
        return static_cast<int>(best);
    }
};
} // namespace hot100::p0084
```

## 复杂度核算

时间 O(n)，每柱至多入栈/出栈一次；辅助栈 O(n)。返回标量，输出 O(1)，无递归。

## C++ 常见陷阱

- 面积乘法在 int 中可能溢出；先用 long long 宽度乘高度。返回 int 依赖官方答案范围，泛化接口应改为 long long。
- 哨兵 i=n 不能压入后再读取 heights[n]；本实现只压真实位置。
- 等高时严格 > 弹栈会保留多个代表，证明要承认某些代表宽度较小，不能声称每个弹出左界都严格更矮。

## 测试与运行

[完整测试源文件](../../../algorithms/cpp/tests/test_p0084.cpp)：穷举长度0到7、高度0到3的全部直方图，与枚举区间最低高度对拍；另测100000根大柱面积并核对输入未修改。

从仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0084.cpp -o /tmp/hot100-cpp/test_p0084
/tmp/hot100-cpp/test_p0084
# 环境支持时增加运行期检查：
g++ -std=c++17 -Wall -Wextra -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0084.cpp -o /tmp/hot100-cpp/test_p0084_san
ASAN_OPTIONS=detect_leaks=0 /tmp/hot100-cpp/test_p0084_san
```

本次环境的LeakSanitizer受ptrace限制，因此命令显式关闭泄漏检测；AddressSanitizer与UndefinedBehaviorSanitizer仍启用。支持泄漏检测的本地环境可移除该环境变量。断言通过说明覆盖输入上的本地结果正确，不能替代证明、完整平台约束核对或在线评测。

## 面试追问

- 如何用 >= 弹栈并让等高柱只保留一个代表？
- 能否用左右边界数组得到相同 O(n) 时间，代价是什么？

## 练习与提示

- 练习1：返回最大矩形的左右边界和高度。提示：结算时保存 left+1、i-1、h。
- 练习2：解只含0/1矩阵最大矩形。提示：把每行上方连续1的长度看成直方图。
