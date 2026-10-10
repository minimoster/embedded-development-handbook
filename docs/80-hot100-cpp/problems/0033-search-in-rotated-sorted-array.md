# 33. 搜索旋转排序数组：C++17 推导与实现

> 分类：二分查找 · 难度：MEDIUM · 官方 Hot 100 顺序：66/100

## 题意与调用约定

数组由严格递增序列旋转而来，元素互不相同。返回目标在当前数组中的下标，不存在返回 -1。空数组是实现额外支持的边界。

题面与最新约束见 [力扣中文官方题目](https://leetcode.cn/problems/search-in-rotated-sorted-array/) / [LeetCode 官方英文题目](https://leetcode.com/problems/search-in-rotated-sorted-array/)。下文给出独立编写的推导、示例与实现；测试是本地验证，没有声称在线评测已通过。

## 从直觉到优化

旋转只制造一个下降断点。任何中点都会把候选区间切成两段，其中至少一段保持有序。先认出有序段，再检查 target 是否落入该段的值域，就能安全丢弃另一半。无需先找到旋转点。

## 不变量与正确性

若目标存在，始终位于闭区间 [lo,hi]。nums[lo]<=nums[mid] 时左段有序，因为唯一下降点不在左段；否则右段有序。对有序段用端点范围判断，保留可能含目标的那一半。中点已单独检验，所以更新可以排除它。

## 算法步骤

1. 在闭区间中计算中点，首先处理相等情况。
2. 比较 nums[lo] 和 nums[mid]，决定哪半段有序。
3. 根据有序半段的闭开值域定位 target。
4. 每轮至少排除中点，直到命中或区间为空。

## 手工追踪

[8,11,15,1,4,6] 找4。lo=0,hi=5,mid=2值15，左段有序但4不在[8,15)，故 lo=3；mid=4值4，返回4。找7时先舍弃左段，再舍弃右侧，最后区间为空。

## 完整 C++17 实现

以下内容来自本课 [可编译头文件](../../../algorithms/cpp/solutions/p0033_search_in_rotated_sorted_array.hpp)，namespace用于让100题可同时进入同一工程。在线提交时一般只保留评测要求的类声明，并按平台要求处理标准库命名；本地辅助头文件负责统一导入标准库。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0033 {
class Solution {
public:
    int search(std::vector<int>& nums, int target) {
        int lo = 0, hi = static_cast<int>(nums.size()) - 1;
        while (lo <= hi) {
            int mid = lo + (hi - lo) / 2;
            if (nums[mid] == target) return mid;
            if (nums[lo] <= nums[mid]) {
                if (nums[lo] <= target && target < nums[mid]) hi = mid - 1;
                else lo = mid + 1;
            } else {
                if (nums[mid] < target && target <= nums[hi]) lo = mid + 1;
                else hi = mid - 1;
            }
        }
        return -1;
    }
};
} // namespace hot100::p0033
```

## 复杂度核算

时间 O(log n)（空输入 O(1)）；额外空间 O(1)，无递归。

## C++ 常见陷阱

- 互不相同是定位有序段的关键；大量重复值时 nums[lo]==nums[mid]==nums[hi] 无法判断，最坏会退化为线性。
- hi 的初始化先转换 size 再减1，避免无符号下溢。
- 范围判断的 < 与 <= 对应中点已排除、边界未排除；随意统一为严格比较会漏边界。

## 测试与运行

[完整测试源文件](../../../algorithms/cpp/tests/test_p0033.cpp)：穷举长度1到30的每个旋转位置和目标区间，对比线性搜索；含未旋转、单项与缺失目标。

从仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0033.cpp -o /tmp/hot100-cpp/test_p0033
/tmp/hot100-cpp/test_p0033
# 环境支持时增加运行期检查：
g++ -std=c++17 -Wall -Wextra -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0033.cpp -o /tmp/hot100-cpp/test_p0033_san
ASAN_OPTIONS=detect_leaks=0 /tmp/hot100-cpp/test_p0033_san
```

本次环境的LeakSanitizer受ptrace限制，因此命令显式关闭泄漏检测；AddressSanitizer与UndefinedBehaviorSanitizer仍启用。支持泄漏检测的本地环境可移除该环境变量。断言通过说明覆盖输入上的本地结果正确，不能替代证明、完整平台约束核对或在线评测。

## 面试追问

- 允许重复值时如何逐步缩小无法判定的区间？
- 能否先找最小值，然后把虚拟有序下标映射回原数组？

## 练习与提示

- 练习1：画出长度2数组的全部旋转与目标位置。提示：特别检查 nums[lo]==nums[mid]。
- 练习2：实现重复值版并构造最坏输入。提示：大量相等值夹一个不同值。
