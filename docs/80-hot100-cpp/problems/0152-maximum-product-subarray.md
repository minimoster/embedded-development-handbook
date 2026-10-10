# 152. 乘积最大子数组：C++17 推导与实现

> 分类：动态规划 · 难度：MEDIUM · 官方 Hot 100 顺序：88/100

## 题意与调用约定

非空数组中求连续非空子数组的最大乘积。负数和零均可能出现；不能选择空子数组来把负答案改成0。官方数据保证子数组乘积处于32位整数范围。

题面与最新约束见 [力扣中文官方题目](https://leetcode.cn/problems/maximum-product-subarray/) / [LeetCode 官方英文题目](https://leetcode.com/problems/maximum-product-subarray/)。下文给出独立编写的推导、示例与实现；测试是本地验证，没有声称在线评测已通过。

## 从直觉到优化

枚举全部区间要 O(n²)。最大和子数组只需一个最好结尾状态，但乘积不同：乘上负数会把最小值变成最大值。因此同时保存“以当前位置结尾”的最大乘积和最小乘积，下一项只需看当前值、旧最大乘当前、旧最小乘当前三者。

## 不变量与正确性

每个以i结尾的非空区间，要么只含nums[i]，要么是以i-1结尾区间追加当前项。正数乘法保持大小次序，负数逆转次序，零全部归零，所以旧最大与旧最小足以表示所有扩展极值。best再对所有结尾最大值取最大。

## 算法步骤

1. 用首项初始化最高、最低和全局最佳，保留非空条件。
2. 先计算旧highest*value、旧lowest*value。
3. 分别从三候选中取最大与最小。
4. 更新全局best，继续扫描。

## 手工追踪

[-2,3,-4]：初始最高最低都-2；乘3后最高3、最低-6；乘-4后候选-4、-12、24，最高24、最低-12，全局24。负数把上一轮最差的-6翻成最好24。零会自然切断之前乘积链。

## 完整 C++17 实现

以下内容来自本课 [可编译头文件](../../../algorithms/cpp/solutions/p0152_maximum_product_subarray.hpp)，namespace用于让100题可同时进入同一工程。在线提交时一般只保留评测要求的类声明，并按平台要求处理标准库命名；本地辅助头文件负责统一导入标准库。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0152 {
class Solution {
public:
    int maxProduct(std::vector<int>& nums) {
        if (nums.empty()) throw std::invalid_argument("maxProduct requires data");
        long long highest = nums[0], lowest = nums[0], best = nums[0];
        for (std::size_t i = 1; i < nums.size(); ++i) {
            long long value = nums[i], fromHigh = highest * value, fromLow = lowest * value;
            highest = std::max({value, fromHigh, fromLow});
            lowest = std::min({value, fromHigh, fromLow});
            best = std::max(best, highest);
        }
        return static_cast<int>(best);
    }
};
} // namespace hot100::p0152
```

## 复杂度核算

时间 O(n)，额外空间 O(1)，无递归和输出容器。

## C++ 常见陷阱

- 必须先保存旧状态的两个乘积，不能更新highest后用新值算lowest。
- 乘法使用 long long 作为缓冲，但任意大数据仍可能溢出；保证依赖题目范围，泛化需要大整数或明确溢出策略。
- best 初始化首元素而不是0，否则单个负数会返回非法空区间的0。

## 测试与运行

[完整测试源文件](../../../algorithms/cpp/tests/test_p0152.cpp)：2500组固定种子含负数/零的短数组，与全部连续区间乘积对拍；含负值翻转、零断开与单个负数。

从仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0152.cpp -o /tmp/hot100-cpp/test_p0152
/tmp/hot100-cpp/test_p0152
# 环境支持时增加运行期检查：
g++ -std=c++17 -Wall -Wextra -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0152.cpp -o /tmp/hot100-cpp/test_p0152_san
ASAN_OPTIONS=detect_leaks=0 /tmp/hot100-cpp/test_p0152_san
```

本次环境的LeakSanitizer受ptrace限制，因此命令显式关闭泄漏检测；AddressSanitizer与UndefinedBehaviorSanitizer仍启用。支持泄漏检测的本地环境可移除该环境变量。断言通过说明覆盖输入上的本地结果正确，不能替代证明、完整平台约束核对或在线评测。

## 面试追问

- 如何返回最佳区间的左右下标？为何最小状态也要跟踪起点？
- 只有正数时这个双状态模型如何简化？

## 练习与提示

- 练习1：增加区间恢复。提示：每个候选携带起点，最高与最低分开保存。
- 练习2：分析 [-2,0,-1] 与 [-2,-1]。提示：前者不允许跨零得到2。
