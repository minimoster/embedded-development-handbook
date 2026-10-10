# 287. 寻找重复数：C++17 推导与实现

> 分类：技巧 · 难度：MEDIUM · 官方 Hot 100 顺序：100/100

## 题意与调用约定

长度n+1，元素在1..n，恰好一个不同的值重复（可以重复多于两次）；返回这个值。不修改数组，只使用常数额外空间。

题面与最新约束见 [力扣中文官方题目](https://leetcode.cn/problems/find-the-duplicate-number/) / [LeetCode 官方英文题目](https://leetcode.com/problems/find-the-duplicate-number/)。下文给出独立编写的推导、示例与实现；测试是本地验证，没有声称在线评测已通过。

## 从直觉到优化

排序会修改输入，哈希会占线性空间。把下标视为节点、nums[i]视为下一节点，数组变成每点一条出边的函数图。从0出发必然进入环。0不会作为任何值出现，所以起点在环外；环入口同时有前驱链与环内前驱，两条不同入边意味着入口值重复。

## 不变量与正确性

快指针每次走2步、慢指针走1步，入环后相对位置每轮移动1模环长，必相遇。若链长μ、环长λ、慢指针相遇已走t步，则t为λ的倍数；相遇点离入口为t-μ模λ。一个指针重置0，再同速走μ步，二者都到入口。唯一重复值保证该入口就是所求值。

## 算法步骤

1. 把slow放nums[0]、fast放nums[nums[0]]，对应从0分别走1/2步。
2. 逐轮慢走1、快走2，直到相遇。
3. 将entrance重置到0，另一指针保持相遇位置。
4. 同时每次走1步，首次再次相遇返回入口值。

## 手工追踪

nums=[3,1,3,4,2] 的0出发路径为0→3→4→2→3，入口3。第一阶段slow=3、fast=4；接着slow=4、fast=3；再slow=2、fast=2相遇。重置指针0，下一步它到3，slow也由2到3，返回3。

## 完整 C++17 实现

以下内容来自本课 [可编译头文件](../../../algorithms/cpp/solutions/p0287_find_the_duplicate_number.hpp)，namespace用于让100题可同时进入同一工程。在线提交时一般只保留评测要求的类声明，并按平台要求处理标准库命名；本地辅助头文件负责统一导入标准库。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0287 {
class Solution {
public:
    int findDuplicate(std::vector<int>& nums) {
        int slow = nums[0], fast = nums[nums[0]];
        while (slow != fast) {
            slow = nums[slow];
            fast = nums[nums[fast]];
        }
        int entrance = 0;
        while (entrance != slow) {
            entrance = nums[entrance];
            slow = nums[slow];
        }
        return entrance;
    }
};
} // namespace hot100::p0287
```

## 复杂度核算

时间 O(n)，两阶段均至多线性步数；额外空间 O(1)，不修改输入、无递归。路径无需存储 visited 集合。

## C++ 常见陷阱

- 每次 nums[nums[fast]] 都依赖值域1..n且长度n+1，任意整数数组不具备下标安全。
- 重置到0要与第一阶段起点0的定义一致；不要混用以nums[0]为起点的另一种证明。
- 检测环相遇点不等于入口，必须做第二阶段。存在多个重复值时题目唯一性证明不能直接照搬。

## 测试与运行

[完整测试源文件](../../../algorithms/cpp/tests/test_p0287.cpp)：穷举n=1到5的所有值域内数组，筛选恰好一个重复值，与频次基线对拍，并核对输入不变；另随机打乱大数组及重复值多于两次。

从仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0287.cpp -o /tmp/hot100-cpp/test_p0287
/tmp/hot100-cpp/test_p0287
# 环境支持时增加运行期检查：
g++ -std=c++17 -Wall -Wextra -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0287.cpp -o /tmp/hot100-cpp/test_p0287_san
ASAN_OPTIONS=detect_leaks=0 /tmp/hot100-cpp/test_p0287_san
```

本次环境的LeakSanitizer受ptrace限制，因此命令显式关闭泄漏检测；AddressSanitizer与UndefinedBehaviorSanitizer仍启用。支持泄漏检测的本地环境可移除该环境变量。断言通过说明覆盖输入上的本地结果正确，不能替代证明、完整平台约束核对或在线评测。

## 面试追问

- 还可以对值域做二分计数，为什么时间是 O(n log n)？
- 环检测如何迁移到链表入口问题，哪些前提不同？

## 练习与提示

- 练习1：实现值域二分并对拍。提示：若<=mid的数量超过mid，重复值在左值域。
- 练习2：允许重复值出现三次以上，并让部分值缺失。提示：构造[2,2,2,2,2]验证“重复不止两次”仍成立。
