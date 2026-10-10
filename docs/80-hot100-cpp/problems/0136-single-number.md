# 136. 只出现一次的数字：C++17 推导与实现

> 分类：技巧 · 难度：EASY · 官方 Hot 100 顺序：96/100

## 题意与调用约定

非空数组里恰有一个值出现一次，其余每个值出现两次，求单独的值。前提中的出现次数决定本算法语义，不能直接用于任意频次。

题面与最新约束见 [力扣中文官方题目](https://leetcode.cn/problems/single-number/) / [LeetCode 官方英文题目](https://leetcode.com/problems/single-number/)。下文给出独立编写的推导、示例与实现；测试是本地验证，没有声称在线评测已通过。

## 从直觉到优化

哈希计数平均 O(n) 时间但要 O(n) 空间，排序 O(n log n) 也改变顺序。异或按位叠加奇偶：同一个值异或两次归零，零与任何值异或不改变它。结合律与交换律允许配对在任意顺序中消去。

## 不变量与正确性

循环累计result等于已处理前缀的所有值异或。扫描完整数组后，每个重复值的一对可通过交换结合归组，x^x=0；最终只剩唯一值与0异或，因此返回它。负数和0也遵循同样的逐位消去性质。

## 算法步骤

1. 累计值从异或单位元0开始。
2. 逐项执行 result ^= value。
3. 遍历结束返回累计值；无需额外集合或排序。

## 手工追踪

[-4,8,3,8,3]。不必手算负数位模式，利用重排性质把异或式改写成 -4^(8^8)^(3^3)=-4。两组抵消后剩单个负值，说明算法不依赖正数前提。

## 完整 C++17 实现

以下内容来自本课 [可编译头文件](../../../algorithms/cpp/solutions/p0136_single_number.hpp)，namespace用于让100题可同时进入同一工程。在线提交时一般只保留评测要求的类声明，并按平台要求处理标准库命名；本地辅助头文件负责统一导入标准库。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0136 {
class Solution {
public:
    int singleNumber(std::vector<int>& nums) {
        int result = 0;
        for (int value : nums) result ^= value;
        return result;
    }
};
} // namespace hot100::p0136
```

## 复杂度核算

时间 O(n)，额外空间 O(1)，返回标量，不修改数组；假定机器int位宽为常数。

## C++ 常见陷阱

- 必须使用 ^，不是表示逻辑的 || 或逻辑不等。
- 若两个值都出现一次，只能得到它们的异或，无法直接选出其中一个。
- 不要用“所有不同值总和的两倍减数组和”，求和可能溢出且需要去重空间。

## 测试与运行

[完整测试源文件](../../../algorithms/cpp/tests/test_p0136.cpp)：1800组固定种子“单值+成对值”输入随机打乱，验证预先记录的单独值；专测0、负数、INT_MIN。

从仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0136.cpp -o /tmp/hot100-cpp/test_p0136
/tmp/hot100-cpp/test_p0136
# 环境支持时增加运行期检查：
g++ -std=c++17 -Wall -Wextra -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0136.cpp -o /tmp/hot100-cpp/test_p0136_san
ASAN_OPTIONS=detect_leaks=0 /tmp/hot100-cpp/test_p0136_san
```

本次环境的LeakSanitizer受ptrace限制，因此命令显式关闭泄漏检测；AddressSanitizer与UndefinedBehaviorSanitizer仍启用。支持泄漏检测的本地环境可移除该环境变量。断言通过说明覆盖输入上的本地结果正确，不能替代证明、完整平台约束核对或在线评测。

## 面试追问

- 两个值只出现一次时，如何按异或结果的一个非零位分组？
- 其余值出现三次时，按位计数模3如何实现？

## 练习与提示

- 练习1：找两个单独值。提示：先求全部异或，选某个不同位，把输入分成两组各自消去。
- 练习2：构造违背次数前提却返回一个貌似合法值的例子。提示：异或只记录每位奇偶，不能验证频率。
