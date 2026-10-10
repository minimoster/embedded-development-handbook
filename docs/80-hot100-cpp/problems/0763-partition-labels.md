# 763. 划分字母区间：C++17 推导与实现

> 分类：贪心算法 · 难度：MEDIUM · 官方 Hot 100 顺序：80/100

## 题意与调用约定

把小写英文字母串切成尽可能多的连续片段，同一种字母只能出现在一个片段中，返回片段长度。片段拼接必须仍是完整原串。

题面与最新约束见 [力扣中文官方题目](https://leetcode.cn/problems/partition-labels/) / [LeetCode 官方英文题目](https://leetcode.com/problems/partition-labels/)。下文给出独立编写的推导、示例与实现；测试是本地验证，没有声称在线评测已通过。

## 从直觉到优化

尝试每个切点再检查后半串是否出现相同字母，会重复扫描。每个字符最后出现位置提前计算后，当前片段至少必须延伸到所有已见字符的最晚终点。扫描位置追上这个终点时，当前片段已完全封闭，可以最早切开。

## 不变量与正确性

end 是当前片段已见字符最后位置的最大值。任何合法切点都不能早于end，否则某字母跨片段。若 i=end，则当前片段所有字母后面不再出现，这个切点合法；选择最早合法切点为后续保留最大剩余范围，能取得最多片段。

## 算法步骤

1. 第一遍记下每个字母的最后下标。
2. 第二遍用当前字母最后下标扩大 end。
3. 当扫描下标等于 end，记录 end-start+1。
4. 下一片段从 i+1 开始，继续同样规则。

## 手工追踪

"abacddce"：a最后2、b最后1，扫描到2可切出"aba"；c最后6，d最后5，下一段直到6切出"cddc"；e独自成段。长度[3,4,1]。在i=3看到c时不能马上切，因为c还在6出现。

## 完整 C++17 实现

以下内容来自本课 [可编译头文件](../../../algorithms/cpp/solutions/p0763_partition_labels.hpp)，namespace用于让100题可同时进入同一工程。在线提交时一般只保留评测要求的类声明，并按平台要求处理标准库命名；本地辅助头文件负责统一导入标准库。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0763 {
class Solution {
public:
    std::vector<int> partitionLabels(std::string s) {
        std::array<int,26> last; last.fill(-1);
        for (int i = 0; i < static_cast<int>(s.size()); ++i) last[s[i]-'a'] = i;
        std::vector<int> answer;
        int start = 0, end = 0;
        for (int i = 0; i < static_cast<int>(s.size()); ++i) {
            end = std::max(end, last[s[i]-'a']);
            if (i == end) { answer.push_back(end-start+1); start = i+1; }
        }
        return answer;
    }
};
} // namespace hot100::p0763
```

## 复杂度核算

时间 O(n+Σ)，Σ=26；辅助表 O(Σ)=O(1)，输出 O(p)，p为片段数且 p<=min(n,26)。按值字符串参数另持有 O(n) 副本。

## C++ 常见陷阱

- 下标 s[i]-'a' 依赖小写英文字母前提；扩展到Unicode不能直接按字节减a。
- 长度要加1；start=end时也形成长度1片段。
- 结果的片段数不是不同字母数，不同字母的覆盖区间可能交叠被迫合并。

## 测试与运行

[完整测试源文件](../../../algorithms/cpp/tests/test_p0763.cpp)：穷举三字母组成的长度1到8字符串，枚举全部切点求最大片段数，并逐字检查跨片段唯一归属及总长度；含全同字母与空串。

从仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0763.cpp -o /tmp/hot100-cpp/test_p0763
/tmp/hot100-cpp/test_p0763
# 环境支持时增加运行期检查：
g++ -std=c++17 -Wall -Wextra -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0763.cpp -o /tmp/hot100-cpp/test_p0763_san
ASAN_OPTIONS=detect_leaks=0 /tmp/hot100-cpp/test_p0763_san
```

本次环境的LeakSanitizer受ptrace限制，因此命令显式关闭泄漏检测；AddressSanitizer与UndefinedBehaviorSanitizer仍启用。支持泄漏检测的本地环境可移除该环境变量。断言通过说明覆盖输入上的本地结果正确，不能替代证明、完整平台约束核对或在线评测。

## 面试追问

- 若字母集无限或为Unicode码点，最后位置表如何变化？
- 本题可视为合并字母区间，怎样证明两个视角一致？

## 练习与提示

- 练习1：改为返回片段字符串。提示：保存起点与长度后 substr，但计入 O(n) 输出字节。
- 练习2：枚举全部切点验证最大片段数。提示：对长度8以内的字符串使用位掩码表示切口。
