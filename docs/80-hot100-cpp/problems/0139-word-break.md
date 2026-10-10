# 139. 单词拆分：C++17 推导与实现

> 分类：动态规划 · 难度：MEDIUM · 官方 Hot 100 顺序：86/100

## 题意与调用约定

判断整个字符串能否拼成字典中的若干单词，同一单词可重复使用；不要求给出拆分。字典词为非空；本实现忽略空词，空字符串扩展可由零个词拼成。

题面与最新约束见 [力扣中文官方题目](https://leetcode.cn/problems/word-break/) / [LeetCode 官方英文题目](https://leetcode.com/problems/word-break/)。下文给出独立编写的推导、示例与实现；测试是本地验证，没有声称在线评测已通过。

## 从直觉到优化

回溯每个可能前缀会多次到达同一个位置。把位置i解释为“前i个字符是否可以完整拆分”，之后的可行性只取决于位置而非如何到达它。本实现枚举字典词作为最后一词，使用 compare 原地比较，避免反复创建 substr 和哈希临时串。

## 不变量与正确性

possible[end] 表示 s[0,end) 能否拆分。若最后词长度l，其前缀必须 possible[end-l]，尾部字符必须完全等于该词。两条件既必要又充分。由于词非空，end-l<end，递增计算已经拥有所需状态。

## 算法步骤

1. possible[0]=true，表示空前缀。
2. 对每个终点end，遍历字典单词。
3. 排除过长或空词，再检查前缀可达与尾部匹配。
4. 发现任一合法最后词即设true并停止本终点枚举。

## 手工追踪

s="ababc"，字典["ab","aba","c"]。前缀2("ab")可达，前缀3("aba")可达；前缀4通过前缀2+"ab"可达；前缀5通过前缀4+"c"可达。"abacb" 的末尾b不构成任何词，所以完整前缀不可达。

## 完整 C++17 实现

以下内容来自本课 [可编译头文件](../../../algorithms/cpp/solutions/p0139_word_break.hpp)，namespace用于让100题可同时进入同一工程。在线提交时一般只保留评测要求的类声明，并按平台要求处理标准库命名；本地辅助头文件负责统一导入标准库。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0139 {
class Solution {
public:
    bool wordBreak(std::string s, std::vector<std::string>& wordDict) {
        std::vector<unsigned char> possible(s.size() + 1, 0);
        possible[0] = 1;
        for (std::size_t end = 1; end <= s.size(); ++end) {
            for (const std::string& word : wordDict) {
                if (!word.empty() && word.size() <= end && possible[end-word.size()] &&
                    s.compare(end-word.size(), word.size(), word) == 0) {
                    possible[end] = 1; break;
                }
            }
        }
        return possible.back() != 0;
    }
};
} // namespace hot100::p0139
```

## 复杂度核算

设N为串长、D为字典词数、W为全部词长度之和，时间上界 O(N(D+W))，比较短路常减少实际工作；不会忽略字符比较成本。辅助DP O(N)，按值字符串参数 O(N)；不建立哈希、不产生 substr 输出。

## C++ 常见陷阱

- possible[i] 使用半开前缀，不能写成含下标i的区间导致状态错位。
- 先检查 word.size()<=end，才能计算 end-word.size()，防止 size_t 下溢。
- vector<bool> 是代理位容器，普通引用语义不同；用 unsigned char 简化学习。

## 测试与运行

[完整测试源文件](../../../algorithms/cpp/tests/test_p0139.cpp)：1400组固定种子短字符串与字典，与无记忆化递归拆分枚举对拍；专测词可重复、末尾无法拼接与空串。

从仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0139.cpp -o /tmp/hot100-cpp/test_p0139
/tmp/hot100-cpp/test_p0139
# 环境支持时增加运行期检查：
g++ -std=c++17 -Wall -Wextra -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0139.cpp -o /tmp/hot100-cpp/test_p0139_san
ASAN_OPTIONS=detect_leaks=0 /tmp/hot100-cpp/test_p0139_san
```

本次环境的LeakSanitizer受ptrace限制，因此命令显式关闭泄漏检测；AddressSanitizer与UndefinedBehaviorSanitizer仍启用。支持泄漏检测的本地环境可移除该环境变量。断言通过说明覆盖输入上的本地结果正确，不能替代证明、完整平台约束核对或在线评测。

## 面试追问

- 字典很大时，Trie 如何从可达位置向后扫描降低重复比较？
- 要返回全部拆分，为什么输出可能指数级，即使判定DP是多项式？

## 练习与提示

- 练习1：返回一组拆分。提示：对可达end保存来源位置与词下标。
- 练习2：用Trie改写，并比较字典总长度与最长词长对复杂度的影响。提示：从每个可达位置沿字符树走。

## 实现复用与边界检查

dp[i] 表示半开前缀 s[0,i)，即前i个字符。substr(j,i-j)、dp[0]=true 与答案 dp[n] 应保持同一状态定义，避免差一位。
