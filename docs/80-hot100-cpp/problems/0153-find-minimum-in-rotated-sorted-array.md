# 153. 寻找旋转排序数组中的最小值：C++17 推导与实现

> 分类：二分查找 · 难度：MEDIUM · 官方 Hot 100 顺序：67/100

## 题意与调用约定

非空数组是严格递增序列的旋转，返回其中最小值。允许旋转后与原数组相同；元素互异。教学实现对不满足非空前提的调用抛出异常。

题面与最新约束见 [力扣中文官方题目](https://leetcode.cn/problems/find-minimum-in-rotated-sorted-array/) / [LeetCode 官方英文题目](https://leetcode.com/problems/find-minimum-in-rotated-sorted-array/)。下文给出独立编写的推导、示例与实现；测试是本地验证，没有声称在线评测已通过。

## 从直觉到优化

线性找最小值很直接。二分需要一个能判断“最小值在哪边”的参照物，右端点正合适。中点大于右端，说明中点仍处于较大的前段，断点在右侧；中点小于右端，则中点及右端之间没有下降，最小值不在中点之后。

## 不变量与正确性

最小值下标始终在 [lo,hi]。若 nums[mid]>nums[hi]，最小值一定严格在 mid 右侧，令 lo=mid+1；否则右侧局部有序，mid 仍可能是最小值，所以令 hi=mid。区间长度每次减少，剩一项时即答案。

## 算法步骤

1. 建立非空闭区间，循环条件使用 lo<hi。
2. 以 nums[hi] 为参照判断中点属于高段还是低段。
3. 高段排除中点及左侧；低段保留中点，收缩右端。
4. 单点收敛后返回对应值。

## 手工追踪

[9,12,2,5,7]：mid=2值2 <=右值7，所以 hi=2；[0,2] 的mid=1值12>2，所以 lo=2。结果2。已排序 [2,5,7] 会不断缩右端，仍正确停在下标0。

## 完整 C++17 实现

以下内容来自本课 [可编译头文件](../../../algorithms/cpp/solutions/p0153_find_minimum_in_rotated_sorted_array.hpp)，namespace用于让100题可同时进入同一工程。在线提交时一般只保留评测要求的类声明，并按平台要求处理标准库命名；本地辅助头文件负责统一导入标准库。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0153 {
class Solution {
public:
    int findMin(std::vector<int>& nums) {
        if (nums.empty()) throw std::invalid_argument("findMin requires a nonempty array");
        int lo = 0, hi = static_cast<int>(nums.size()) - 1;
        while (lo < hi) {
            int mid = lo + (hi - lo) / 2;
            if (nums[mid] > nums[hi]) lo = mid + 1;
            else hi = mid;
        }
        return nums[lo];
    }
};
} // namespace hot100::p0153
```

## 复杂度核算

时间 O(log n)，额外空间 O(1)。不修改输入，不分配输出容器。

## C++ 常见陷阱

- 写 hi=mid-1 会在中点就是最小值时删掉答案。
- 比较对象是当前 nums[hi]，不是固定 nums.back()；证明依赖当前区间。
- 重复值版遇到相等通常只能 --hi，最坏 O(n)；空输入不能随意返回 -1，因为 -1可能是合法最小值。

## 测试与运行

[完整测试源文件](../../../algorithms/cpp/tests/test_p0153.cpp)：穷举长度1到40的所有旋转，验证最小值与输入未改变；含单项及空输入异常。

从仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0153.cpp -o /tmp/hot100-cpp/test_p0153
/tmp/hot100-cpp/test_p0153
# 环境支持时增加运行期检查：
g++ -std=c++17 -Wall -Wextra -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0153.cpp -o /tmp/hot100-cpp/test_p0153_san
ASAN_OPTIONS=detect_leaks=0 /tmp/hot100-cpp/test_p0153_san
```

本次环境的LeakSanitizer受ptrace限制，因此命令显式关闭泄漏检测；AddressSanitizer与UndefinedBehaviorSanitizer仍启用。支持泄漏检测的本地环境可移除该环境变量。断言通过说明覆盖输入上的本地结果正确，不能替代证明、完整平台约束核对或在线评测。

## 面试追问

- 找旋转次数相当于寻找什么下标？
- 允许重复值时为什么不能承诺对数最坏复杂度？

## 练习与提示

- 练习1：把接口改为返回最小值下标。提示：返回收敛后的 lo。
- 练习2：为 [2,2,2,0,2] 设计重复值版本。提示：相等时只缩去一项，不能删半段。
