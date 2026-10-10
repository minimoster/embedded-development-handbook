# 35. 搜索插入位置：C++17 推导与实现

> 分类：二分查找 · 难度：EASY · 官方 Hot 100 顺序：63/100

## 题意与调用约定

有序整数数组中，返回第一个不小于 target 的下标；若所有元素都小于它，返回数组长度。官方数组严格递增；本实现也自然适用于重复值，并把空数组作为教学扩展。

题面与最新约束见 [力扣中文官方题目](https://leetcode.cn/problems/search-insert-position/) / [LeetCode 官方英文题目](https://leetcode.com/problems/search-insert-position/)。下文给出独立编写的推导、示例与实现；测试是本地验证，没有声称在线评测已通过。

## 从直觉到优化

线性扫描是在寻找一条“左侧都小于 target、右侧都不小于 target”的分界。排序让判断具有单调性：若中点仍小于 target，左半段都不可能是答案；否则中点必须保留在候选边界中。于是二分找边界，而不是只查是否相等。

## 不变量与正确性

循环始终保持 nums[0,lo) 全部小于 target，nums[hi,n) 全部不小于 target；未知区间为 [lo,hi)。更新 lo=mid+1 或 hi=mid 都保留该性质且严格缩短区间。结束时 lo=hi，唯一分界就是插入位置。

## 算法步骤

1. 建立半开区间 [0,n)，允许最终位置为 n。
2. 若 nums[mid]<target，把中点及其左侧排除。
3. 否则将 hi 缩到 mid，使相等元素仍在答案候选中。
4. 返回 lo，无需另写“没找到”的分支。

## 手工追踪

nums=[2,5,8,13]，target=6。起点 [0,4)，mid=2 值8，变成 [0,2)；mid=1 值5，变成 [2,2)。返回2，插入后 [2,5,6,8,13]。target=20 时 lo 最终会到4，正好是尾后位置。

## 完整 C++17 实现

以下内容来自本课 [可编译头文件](../../../algorithms/cpp/solutions/p0035_search_insert_position.hpp)，namespace用于让100题可同时进入同一工程。在线提交时一般只保留评测要求的类声明，并按平台要求处理标准库命名；本地辅助头文件负责统一导入标准库。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0035 {
class Solution {
public:
    int searchInsert(std::vector<int>& nums, int target) {
        int lo = 0, hi = static_cast<int>(nums.size());
        while (lo < hi) {
            int mid = lo + (hi - lo) / 2;
            if (nums[mid] < target) lo = mid + 1;
            else hi = mid;
        }
        return lo;
    }
};
} // namespace hot100::p0035
```

## 复杂度核算

时间 O(log(n+1))；额外空间 O(1)，无递归栈、无输出容器。

## C++ 常见陷阱

- 不要把 hi=n 与 while(lo<=hi) 混用，否则会读取 nums[n]。
- 中点写 lo+(hi-lo)/2，避免 lo+hi 溢出。
- nums.size() 是无符号数；先转为 int，且约定 n 不超过 INT_MAX，避免空数组 size()-1 下溢。

## 测试与运行

[完整测试源文件](../../../algorithms/cpp/tests/test_p0035.cpp)：穷举9个候选值的全部有序子集，并逐目标对比线性边界；包含尾后插入、空数组与缺失值。

从仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0035.cpp -o /tmp/hot100-cpp/test_p0035
/tmp/hot100-cpp/test_p0035
# 环境支持时增加运行期检查：
g++ -std=c++17 -Wall -Wextra -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0035.cpp -o /tmp/hot100-cpp/test_p0035_san
ASAN_OPTIONS=detect_leaks=0 /tmp/hot100-cpp/test_p0035_san
```

本次环境的LeakSanitizer受ptrace限制，因此命令显式关闭泄漏检测；AddressSanitizer与UndefinedBehaviorSanitizer仍启用。支持泄漏检测的本地环境可移除该环境变量。断言通过说明覆盖输入上的本地结果正确，不能替代证明、完整平台约束核对或在线评测。

## 面试追问

- 如何改为第一个大于 target 的位置？把比较条件改成 nums[mid]<=target。
- 数据位于链表上时，为什么二分无法获得同样的访问成本？

## 练习与提示

- 练习1：返回 target 的出现次数。提示：分别求 lower_bound 和 upper_bound，再相减。
- 练习2：为二分写循环断言。提示：检查两个已知区间是否满足各自不等式。
