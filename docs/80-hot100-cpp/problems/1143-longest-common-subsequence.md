# 1143. 最长公共子序列：C++17 推导与实现

> 分类：多维动态规划 · 难度：MEDIUM · 官方 Hot 100 顺序：94/100

## 题意与调用约定

两个字符串中找最长公共子序列的长度；子序列可不连续但必须保持每个原串内的相对顺序。返回长度，不输出具体序列。

题面与最新约束见 [力扣中文官方题目](https://leetcode.cn/problems/longest-common-subsequence/) / [LeetCode 官方英文题目](https://leetcode.com/problems/longest-common-subsequence/)。下文给出独立编写的推导、示例与实现；测试是本地验证，没有声称在线评测已通过。

## 从直觉到优化

枚举一串的所有子序列要指数时间。设dp[i][j]为两个前缀的LCS：末字符相同时可把它加到两个更短前缀的LCS；不同时至少有一方末字符不被使用，取删掉一方后的最大值。当前行只依赖上行、左格和左上格，滚动时单独保存左上旧值。

## 不变量与正确性

内层j更新前，best[j]是旧上方 dp[i-1][j]，best[j-1]是新左方 dp[i][j-1]，diagonal是旧左上 dp[i-1][j-1]。保存above后再写best[j]，最后把above移给diagonal，不会丢失下一格所需的旧状态。

## 算法步骤

1. 把短字符串作为列，零行与零列初始化0。
2. 每行开始 diagonal=0。
3. 先保存 above；相等取 diagonal+1，否则 max(above,left)。
4. 更新diagonal为刚保存的旧above。
5. 最后返回滚动数组末项。

## 手工追踪

text1="axbyc"，text2="abc"。逐行DP（去掉0列）为[1,1,1]、[1,1,1]、[1,2,2]、[1,2,2]、[1,2,3]。a、b、c被按原顺序匹配，跳过x、y，得到3。

## 完整 C++17 实现

以下内容来自本课 [可编译头文件](../../../algorithms/cpp/solutions/p1143_longest_common_subsequence.hpp)，namespace用于让100题可同时进入同一工程。在线提交时一般只保留评测要求的类声明，并按平台要求处理标准库命名；本地辅助头文件负责统一导入标准库。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p1143 {
class Solution {
public:
    int longestCommonSubsequence(std::string text1, std::string text2) {
        if (text2.size() > text1.size()) std::swap(text1, text2);
        std::vector<int> best(text2.size() + 1, 0);
        for (char a : text1) {
            int diagonal = 0;
            for (std::size_t j = 1; j <= text2.size(); ++j) {
                int above = best[j];
                if (a == text2[j-1]) best[j] = diagonal + 1;
                else best[j] = std::max(best[j], best[j-1]);
                diagonal = above;
            }
        }
        return best.back();
    }
};
} // namespace hot100::p1143
```

## 复杂度核算

时间 O(mn)，算法辅助空间 O(min(m,n))；接口按值持有两串，合计参数副本 O(m+n)。返回标量，无递归。

## C++ 常见陷阱

- diagonal必须是旧行左上值，不能直接用已更新 best[j-1]。
- 相等时长度加1只匹配一次当前字符，重复字符不能无限复用。
- 若需恢复LCS，一行压缩丢弃决策；需完整表、分治恢复或额外结构。

## 测试与运行

[完整测试源文件](../../../algorithms/cpp/tests/test_p1143.cpp)：1500组固定种子短双字符串，枚举一串全部子序列并在另一串匹配，取最长作为基线；含空串、无共同字符和重复字符。

从仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p1143.cpp -o /tmp/hot100-cpp/test_p1143
/tmp/hot100-cpp/test_p1143
# 环境支持时增加运行期检查：
g++ -std=c++17 -Wall -Wextra -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p1143.cpp -o /tmp/hot100-cpp/test_p1143_san
ASAN_OPTIONS=detect_leaks=0 /tmp/hot100-cpp/test_p1143_san
```

本次环境的LeakSanitizer受ptrace限制，因此命令显式关闭泄漏检测；AddressSanitizer与UndefinedBehaviorSanitizer仍启用。支持泄漏检测的本地环境可移除该环境变量。断言通过说明覆盖输入上的本地结果正确，不能替代证明、完整平台约束核对或在线评测。

## 面试追问

- 最长公共子串为何使用完全不同的不相等转移？
- 怎样用 Hirschberg 分治在较低空间恢复实际公共子序列？

## 练习与提示

- 练习1：改为最长公共子串。提示：状态是以两个当前位置共同结尾，不相等要清零。
- 练习2：保存二维表并恢复一个LCS。提示：相等走左上，否则走值较大的一侧。
