# 34. 在排序数组中查找元素的第一个和最后一个位置：C++17 推导与实现

> 分类：二分查找 · 难度：MEDIUM · 官方 Hot 100 顺序：65/100

## 题意与调用约定

在非递减数组中返回目标值首次和末次出现的下标，不存在时返回 [-1,-1]。重复值可能占据很长区间，要求对数时间。

题面与最新约束见 [力扣中文官方题目](https://leetcode.cn/problems/find-first-and-last-position-of-element-in-sorted-array/) / [LeetCode 官方英文题目](https://leetcode.com/problems/find-first-and-last-position-of-element-in-sorted-array/)。下文给出独立编写的推导、示例与实现；测试是本地验证，没有声称在线评测已通过。

## 从直觉到优化

找到任意一个目标后向两边扫描，在全相等数组上退化成 O(n)。更好的方法是分别寻找两个单调分界：第一个 >=target 的位置与第一个 >target 的位置。目标区间就是 [lower,upper)，末位置为 upper-1。

## 不变量与正确性

bound 的 upper=false 对应 lower_bound；upper=true 对应 upper_bound。两者都维持“lo 左侧符合跳过条件、hi 右侧不符合”的不变量。若 lower 指向目标，则两边界之间的所有值都等于目标，边界外不会再有目标。

## 算法步骤

1. 使用同一二分函数，通过布尔参数选择是否跳过相等值。
2. 先计算 lower；若越界或不相等，立即返回缺失标记。
3. 再计算 upper，返回 lower 与 upper-1。

## 手工追踪

数组 [1,3,3,3,8] 中找3。lower 的比较把相等元素保留，停在1；upper 把相等元素排除，停在4。答案 [1,3]。若找2，lower=1但 nums[1]=3，直接判为不存在。

## 完整 C++17 实现

以下内容来自本课 [可编译头文件](../../../algorithms/cpp/solutions/p0034_find_first_and_last_position_of_element_in_sorted_array.hpp)，namespace用于让100题可同时进入同一工程。在线提交时一般只保留评测要求的类声明，并按平台要求处理标准库命名；本地辅助头文件负责统一导入标准库。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0034 {
class Solution {
    int bound(const std::vector<int>& nums, int target, bool upper) {
        int lo = 0, hi = static_cast<int>(nums.size());
        while (lo < hi) {
            int mid = lo + (hi - lo) / 2;
            if (nums[mid] < target || (upper && nums[mid] == target)) lo = mid + 1;
            else hi = mid;
        }
        return lo;
    }
public:
    std::vector<int> searchRange(std::vector<int>& nums, int target) {
        int first = bound(nums, target, false);
        if (first == static_cast<int>(nums.size()) || nums[first] != target) return {-1,-1};
        return {first, bound(nums, target, true) - 1};
    }
};
} // namespace hot100::p0034
```

## 复杂度核算

两次二分共 O(log(n+1)) 时间；额外空间 O(1)。返回固定两个整数，输出空间 O(1)。

## C++ 常见陷阱

- 不要使用 upper_bound(target+1) 代替 upper_bound(target)：target=INT_MAX 时加一会溢出。
- 缺失检测必须先看 first==n，再读 nums[first]。
- 闭区间答案与半开搜索区间不是同一个概念，upper 本身不属于目标区间。

## 测试与运行

[完整测试源文件](../../../algorithms/cpp/tests/test_p0034.cpp)：1000组固定种子非递减数组，逐目标与线性首末下标对拍；含重复值、空数组、INT_MIN/INT_MAX。

从仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0034.cpp -o /tmp/hot100-cpp/test_p0034
/tmp/hot100-cpp/test_p0034
# 环境支持时增加运行期检查：
g++ -std=c++17 -Wall -Wextra -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0034.cpp -o /tmp/hot100-cpp/test_p0034_san
ASAN_OPTIONS=detect_leaks=0 /tmp/hot100-cpp/test_p0034_san
```

本次环境的LeakSanitizer受ptrace限制，因此命令显式关闭泄漏检测；AddressSanitizer与UndefinedBehaviorSanitizer仍启用。支持泄漏检测的本地环境可移除该环境变量。断言通过说明覆盖输入上的本地结果正确，不能替代证明、完整平台约束核对或在线评测。

## 面试追问

- 如何在磁盘索引中查找某个键的完整记录区间？
- 若要找所有位于 [a,b] 的元素，两个边界如何组合？

## 练习与提示

- 练习1：求非递减数组里目标的频次。提示：upper-lower；缺失时也是0。
- 练习2：改写为两个独立函数并证明边界。提示：分别写出 < 与 <= 的已知前缀性质。
