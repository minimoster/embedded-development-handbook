# 31. 下一个排列：C++17 推导与实现

> 分类：技巧 · 难度：MEDIUM · 官方 Hot 100 顺序：99/100

## 题意与调用约定

原地将数组变为字典序紧邻的下一个排列；已是最大排列则回到最小升序排列。元素可重复，不输出所有排列；空数组与单元素保持原样。

题面与最新约束见 [力扣中文官方题目](https://leetcode.cn/problems/next-permutation/) / [LeetCode 官方英文题目](https://leetcode.com/problems/next-permutation/)。下文给出独立编写的推导、示例与实现；测试是本地验证，没有声称在线评测已通过。

## 从直觉到优化

枚举全部排列并排序在长度稍大时不可行。字典序最小增大应尽量保留左前缀，因此从右寻找还能增大的最右位置。它后面的区间已非递增，是该前缀下最大后缀；给pivot换成稍大的最小值，再把后缀排成最小，就得到紧邻后继。

## 不变量与正确性

pivot右侧后缀非递增。若不存在pivot，全数组已为最大排列，逆序即最小排列。否则从右找首个严格大于pivot的值，它是后缀中大于pivot的最小候选；交换后后缀仍非递增，反转成非递减得到最小尾部。任何更早增大的方案更大，任何同pivot且更大替换也更大。

## 算法步骤

1. 长度不足2直接返回，确保任何迭代器构造都处于合法范围。
2. 从n-2向左跳过 nums[i]>=nums[i+1]，定位pivot。
3. 有pivot时从右找首个 >nums[pivot] 的后继。
4. 交换pivot与后继，只提升最右可提升位。
5. 反转pivot之后的整个后缀；无pivot则反转全部。

## 手工追踪

[1,4,3,2] 的非递增后缀为[4,3,2]，pivot=1。从右找到最小较大值2，交换得[2,4,3,1]；反转后缀得[2,1,3,4]，比原排列大且最接近。[3,2,1]无pivot，直接变[1,2,3]。

## 完整 C++17 实现

以下内容来自本课 [可编译头文件](../../../algorithms/cpp/solutions/p0031_next_permutation.hpp)，namespace用于让100题可同时进入同一工程。在线提交时一般只保留评测要求的类声明，并按平台要求处理标准库命名；本地辅助头文件负责统一导入标准库。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0031 {
class Solution {
public:
    void nextPermutation(std::vector<int>& nums) {
        if (nums.size() < 2) return;
        int n = static_cast<int>(nums.size()), pivot = n - 2;
        while (pivot >= 0 && nums[pivot] >= nums[pivot+1]) --pivot;
        if (pivot >= 0) {
            int successor = n - 1;
            while (nums[successor] <= nums[pivot]) --successor;
            std::swap(nums[pivot], nums[successor]);
        }
        std::reverse(nums.begin() + (pivot + 1), nums.end());
    }
};
} // namespace hot100::p0031
```

## 复杂度核算

时间 O(n)，额外空间 O(1)，原地修改输入。std::reverse 用常数临时空间，不排序后缀；无递归。

## C++ 常见陷阱

- 重复值情况下pivot条件是 >=，后继条件是严格 >，写错会停在相等值。
- 空与单项先返回；否则空数组pivot=-2会让begin()+(pivot+1)成为非法begin()-1。有符号pivot用于其他输入的-1状态。
- 末尾后缀本就有序反向，直接reverse够了，调用sort会不必要地提高时间。

## 测试与运行

[完整测试源文件](../../../algorithms/cpp/tests/test_p0031.cpp)：穷举长度0到9的三值数组，与std::next_permutation对拍；专测降序回绕、重复值、空数组及单项。

从仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0031.cpp -o /tmp/hot100-cpp/test_p0031
/tmp/hot100-cpp/test_p0031
# 环境支持时增加运行期检查：
g++ -std=c++17 -Wall -Wextra -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0031.cpp -o /tmp/hot100-cpp/test_p0031_san
ASAN_OPTIONS=detect_leaks=0 /tmp/hot100-cpp/test_p0031_san
```

本次环境的LeakSanitizer受ptrace限制，因此命令显式关闭泄漏检测；AddressSanitizer与UndefinedBehaviorSanitizer仍启用。支持泄漏检测的本地环境可移除该环境变量。断言通过说明覆盖输入上的本地结果正确，不能替代证明、完整平台约束核对或在线评测。

## 面试追问

- 如何实现字典序上一个排列？
- 从最小排列反复调用可以枚举多少个不同排列，重复元素影响公式吗？

## 练习与提示

- 练习1：实现 prevPermutation。提示：反转所有不等号，并将后缀整理成最大。
- 练习2：对包含重复值的小数组与 std::next_permutation 对拍。提示：标准函数false时也会重排回最小。
