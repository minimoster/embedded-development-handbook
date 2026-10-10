# 169. 多数元素：C++17 推导与实现

> 分类：技巧 · 难度：EASY · 官方 Hot 100 顺序：97/100

## 题意与调用约定

非空数组保证存在出现次数严格超过一半的值，返回它。不是“出现次数最多”而是“超过 n/2”；若去掉存在保证，必须增加验证。

题面与最新约束见 [力扣中文官方题目](https://leetcode.cn/problems/majority-element/) / [LeetCode 官方英文题目](https://leetcode.com/problems/majority-element/)。下文给出独立编写的推导、示例与实现；测试是本地验证，没有声称在线评测已通过。

## 从直觉到优化

计数哈希与排序都能找，但分别需线性空间或排序时间。把两种不同值成对删除：严格多数即使每次与非多数配对，也不可能全部消失。Boyer-Moore 用候选与票数压缩这项消去过程。

## 不变量与正确性

已处理前缀可分成若干不同值的消去对与 votes 个 candidate 剩余项。相同值增加剩余票，异值消去一票，票归零后下一值重新选候选。删除异值对保持“原严格多数仍为剩余集合严格多数”，因此完整扫描的候选必是多数。

## 算法步骤

1. votes初始化0，candidate任意初值。
2. 票为0时先用当前值建立新候选。
3. 相同则加1，不同则减1。
4. 在多数存在前提下，返回最后候选。

## 手工追踪

[5,1,5,2,5,5]：候选5票1；遇1票归0；再选5票1；遇2归0；最后两个5给票2。消去了(5,1)与(5,2)，剩下5、5，所以候选5。最后票数2不是值5在完整数组中的次数4。

## 完整 C++17 实现

以下内容来自本课 [可编译头文件](../../../algorithms/cpp/solutions/p0169_majority_element.hpp)，namespace用于让100题可同时进入同一工程。在线提交时一般只保留评测要求的类声明，并按平台要求处理标准库命名；本地辅助头文件负责统一导入标准库。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0169 {
class Solution {
public:
    int majorityElement(std::vector<int>& nums) {
        if (nums.empty()) throw std::invalid_argument("majority requires data");
        int candidate = 0, votes = 0;
        for (int value : nums) {
            if (votes == 0) candidate = value;
            votes += value == candidate ? 1 : -1;
        }
        return candidate;
    }
};
} // namespace hot100::p0169
```

## 复杂度核算

时间 O(n)，额外空间 O(1)，不修改数组；返回标量。若需验证多数存在，再扫描一次仍为 O(n) 时间。

## C++ 常见陷阱

- votes 是净剩余票数，不是候选总频次。
- 必须在 votes==0 时重选候选，别把 candidate初值0误当真实先验。
- 无多数时算法仍返回某个候选，不能据此宣称它超过一半。

## 测试与运行

[完整测试源文件](../../../algorithms/cpp/tests/test_p0169.cpp)：2500组固定种子输入保证严格多数，打乱后验证候选并重新计数核实前提；含单项INT_MIN及多轮消票。

从仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0169.cpp -o /tmp/hot100-cpp/test_p0169
/tmp/hot100-cpp/test_p0169
# 环境支持时增加运行期检查：
g++ -std=c++17 -Wall -Wextra -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0169.cpp -o /tmp/hot100-cpp/test_p0169_san
ASAN_OPTIONS=detect_leaks=0 /tmp/hot100-cpp/test_p0169_san
```

本次环境的LeakSanitizer受ptrace限制，因此命令显式关闭泄漏检测；AddressSanitizer与UndefinedBehaviorSanitizer仍启用。支持泄漏检测的本地环境可移除该环境变量。断言通过说明覆盖输入上的本地结果正确，不能替代证明、完整平台约束核对或在线评测。

## 面试追问

- 无多数保证时如何返回 optional<int>？
- 寻找出现次数超过 n/3 的元素，需要几个候选槽？

## 练习与提示

- 练习1：加入第二遍频次验证。提示：严格比较 count>n/2，注意偶数与奇数。
- 练习2：实现两个候选的 n/3 版本。提示：第三种值到来且两槽都非空时同时消票，最终仍需验证。
