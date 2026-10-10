# 32. 最长有效括号：C++17 推导与实现

> 分类：动态规划 · 难度：HARD · 官方 Hot 100 顺序：90/100

## 题意与调用约定

只含圆括号的字符串，求最长连续有效括号子串长度。子串必须连续，不能删除字符再匹配；若没有返回0。

题面与最新约束见 [力扣中文官方题目](https://leetcode.cn/problems/longest-valid-parentheses/) / [LeetCode 官方英文题目](https://leetcode.com/problems/longest-valid-parentheses/)。下文给出独立编写的推导、示例与实现；测试是本地验证，没有声称在线评测已通过。

## 从直觉到优化

枚举所有起点并向右检验余额需要 O(n²)。令 ending[i] 为恰好以i结尾的最长有效子串。只有右括号能结束有效串；先跨过 ending[i-1] 已知的有效段，再看前一项是否是可匹配当前右括号的左括号，匹配后还能接上更早的有效段。

## 不变量与正确性

opening=i-1-ending[i-1] 是当前右括号潜在匹配位置：中间部分必须是以i-1结尾的最长有效后缀。若该处为左括号，包围已有有效段得到长度 ending[i-1]+2，再加 ending[opening-1] 接上相邻有效前缀。若潜在位置不存在或非左括号，当前没有有效结尾。

## 算法步骤

1. ending全置0，扫描下标1到n-1。
2. 左括号不可能作为有效串末端，跳过。
3. 对右括号计算跨过已知有效后缀后的opening。
4. 若匹配，加入2及opening之前相邻有效段，更新全局最大。

## 手工追踪

"()(())"：i=1 的 ) 与0匹配，ending[1]=2；i=4的 ) 与3匹配，ending[4]=2；i=5的 ) 跨过这2项，opening=2为(，得到4，再接 ending[1]=2，合成6。这同时处理嵌套与并列。

## 完整 C++17 实现

以下内容来自本课 [可编译头文件](../../../algorithms/cpp/solutions/p0032_longest_valid_parentheses.hpp)，namespace用于让100题可同时进入同一工程。在线提交时一般只保留评测要求的类声明，并按平台要求处理标准库命名；本地辅助头文件负责统一导入标准库。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0032 {
class Solution {
public:
    int longestValidParentheses(std::string s) {
        int n = static_cast<int>(s.size()), best = 0;
        std::vector<int> ending(n, 0);
        for (int i = 1; i < n; ++i) {
            if (s[i] != ')') continue;
            int opening = i - 1 - ending[i-1];
            if (opening >= 0 && s[opening] == '(') {
                ending[i] = ending[i-1] + 2;
                if (opening > 0) ending[i] += ending[opening-1];
                best = std::max(best, ending[i]);
            }
        }
        return best;
    }
};
} // namespace hot100::p0032
```

## 复杂度核算

时间 O(n)，辅助DP空间 O(n)，字符串按值参数 O(n)。无递归。可用双向计数法降至 O(1) 算法额外空间，但需两遍扫描。

## C++ 常见陷阱

- opening 必须先 >=0 才能索引；i-1-ending 可能是-1。
- ending[i]表示结尾后缀，不是前缀中的全局最大值；混淆会使跳过长度错误。
- 连接位置为 opening-1，不是 i-ending[i-1]-1，画出下标避免差一位。

## 测试与运行

[完整测试源文件](../../../algorithms/cpp/tests/test_p0032.cpp)：穷举长度0到15的全部圆括号字符串，与每个起点余额扫描的独立基线比较；覆盖嵌套、连接与无有效串。

从仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0032.cpp -o /tmp/hot100-cpp/test_p0032
/tmp/hot100-cpp/test_p0032
# 环境支持时增加运行期检查：
g++ -std=c++17 -Wall -Wextra -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0032.cpp -o /tmp/hot100-cpp/test_p0032_san
ASAN_OPTIONS=detect_leaks=0 /tmp/hot100-cpp/test_p0032_san
```

本次环境的LeakSanitizer受ptrace限制，因此命令显式关闭泄漏检测；AddressSanitizer与UndefinedBehaviorSanitizer仍启用。支持泄漏检测的本地环境可移除该环境变量。断言通过说明覆盖输入上的本地结果正确，不能替代证明、完整平台约束核对或在线评测。

## 面试追问

- 用栈与“上一次失败位置”能否得到相同 O(n) 时间？
- 仅从左向右计数为什么漏掉 "(()"，逆向扫描怎样补足？

## 练习与提示

- 练习1：实现栈版对拍。提示：以-1作初始边界，失败右括号重设边界。
- 练习2：返回最长区间而非长度。提示：更新best时起点为 i-ending[i]+1。
