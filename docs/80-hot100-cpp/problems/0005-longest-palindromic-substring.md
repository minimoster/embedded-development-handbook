# 5. 最长回文子串：C++17 推导与实现

> 分类：多维动态规划 · 难度：MEDIUM · 官方 Hot 100 顺序：93/100

## 题意与调用约定

返回原串的一段最长连续回文子串；多个最优答案均合法。本实现约定并列时返回起点最靠左者，空串扩展返回空串。

题面与最新约束见 [力扣中文官方题目](https://leetcode.cn/problems/longest-palindromic-substring/) / [LeetCode 官方英文题目](https://leetcode.com/problems/longest-palindromic-substring/)。下文给出独立编写的推导、示例与实现；测试是本地验证，没有声称在线评测已通过。

## 从直觉到优化

枚举 O(n²) 个区间再逐个扫描回文会到 O(n³)。回文有“剥两端”的结构：两端相等且内部为回文，整个区间就是回文。按区间长度递增计算二维布尔状态，避免反复检查内部字符。

## 不变量与正确性

palindrome[l][r] 表示闭区间 [l,r] 是否回文。长度1天然成立，长度2只需端点相等；更长区间依赖长度减2的内部区间。长度递增保证依赖已完成。扫描左端递增且只在更长时更新，使同长度并列保留最左答案。

## 算法步骤

1. 空串单独返回；分配n×n布尔状态。
2. 按length=1..n枚举，再枚举合法left。
3. 判断端点相等且长度<=2或内部回文。
4. 成功时写状态，并按严格长度增大更新起点与长度。
5. 最终一次 substr 生成结果。

## 手工追踪

"xabbaq"：长度1全部为回文；长度2发现"bb"；长度4区间[1,4]两端a相等、内部[2,3]为bb，所以"abba"成立。其他更长候选端点或内部不满足，返回abba。

## 完整 C++17 实现

以下内容来自本课 [可编译头文件](../../../algorithms/cpp/solutions/p0005_longest_palindromic_substring.hpp)，namespace用于让100题可同时进入同一工程。在线提交时一般只保留评测要求的类声明，并按平台要求处理标准库命名；本地辅助头文件负责统一导入标准库。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0005 {
class Solution {
public:
    std::string longestPalindrome(std::string s) {
        int n = static_cast<int>(s.size());
        if (!n) return "";
        std::vector<std::vector<unsigned char>> palindrome(n, std::vector<unsigned char>(n, 0));
        int bestStart = 0, bestLength = 1;
        for (int length = 1; length <= n; ++length) {
            for (int left = 0; left + length <= n; ++left) {
                int right = left + length - 1;
                if (s[left] == s[right] && (length <= 2 || palindrome[left+1][right-1])) {
                    palindrome[left][right] = 1;
                    if (length > bestLength) { bestStart = left; bestLength = length; }
                }
            }
        }
        return s.substr(bestStart, bestLength);
    }
};
} // namespace hot100::p0005
```

## 复杂度核算

时间 Θ(n²)，辅助表 Θ(n²)，参数字符串副本 O(n)，输出 O(L)（L为最长回文长度）。最终 substr 复制 O(L) 已包含在总时间界内；无递归。

## C++ 常见陷阱

- length<=2 放在 || 左边利用短路，避免读空内部的错位状态。
- unsigned char 状态比 vector<bool> 更容易获得普通值语义。
- 本实现按字节比较；UTF-8中文字符多字节，若要求字符级回文应先转码点。
- 只在末尾取一次 substr；循环中不断生成候选字符串会增加复制成本。

## 测试与运行

[完整测试源文件](../../../algorithms/cpp/tests/test_p0005.cpp)：穷举长度0到10的二字母串，与枚举所有子串的回文检查对拍，同时验证最左并列规则；含奇偶中心、全同字符与空串。

从仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0005.cpp -o /tmp/hot100-cpp/test_p0005
/tmp/hot100-cpp/test_p0005
# 环境支持时增加运行期检查：
g++ -std=c++17 -Wall -Wextra -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0005.cpp -o /tmp/hot100-cpp/test_p0005_san
ASAN_OPTIONS=detect_leaks=0 /tmp/hot100-cpp/test_p0005_san
```

本次环境的LeakSanitizer受ptrace限制，因此命令显式关闭泄漏检测；AddressSanitizer与UndefinedBehaviorSanitizer仍启用。支持泄漏检测的本地环境可移除该环境变量。断言通过说明覆盖输入上的本地结果正确，不能替代证明、完整平台约束核对或在线评测。

## 面试追问

- 中心扩展可在 O(n²) 时间下把辅助空间降到 O(1)，适用于哪些中心？
- Manacher 算法为什么能达到线性？半径复用依赖什么对称关系？

## 练习与提示

- 练习1：实现奇偶中心扩展，按最长和最左规则对拍。提示：中心可为字符或两字符之间。
- 练习2：统计所有回文子串数量。提示：每个true区间计一次，别把相同文字但不同位置合并。
