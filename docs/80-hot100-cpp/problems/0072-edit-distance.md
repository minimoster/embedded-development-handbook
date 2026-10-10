# 72. 编辑距离：C++17 推导与实现

> 分类：多维动态规划 · 难度：MEDIUM · 官方 Hot 100 顺序：95/100

## 题意与调用约定

用插入、删除、替换单个字符，把一个字符串变成另一个，求最少操作数；每种操作成本1。空串转换为长度k的串需要k次插入。

题面与最新约束见 [力扣中文官方题目](https://leetcode.cn/problems/edit-distance/) / [LeetCode 官方英文题目](https://leetcode.com/problems/edit-distance/)。下文给出独立编写的推导、示例与实现；测试是本地验证，没有声称在线评测已通过。

## 从直觉到优化

回溯所有操作会反复到达同一对前缀。dp[i][j]记录把源串前i字符变成目标前j字符的代价。末字符相等可直接继承左上；否则最后一次操作是删、插或替换，分别来自上、左、左上，代价加1。只要保存三个邻居即可滚动。

## 不变量与正确性

内层前，distance[j]为旧上方，distance[j-1]为本行左方，diagonal为旧左上。删除源末字符对应above+1，插入目标末字符对应left+1，替换对应diagonal+1。操作分支覆盖最优编辑对齐的末步，取最小保证最优子结构。

## 算法步骤

1. 短串作列，distance[j]=j 表示空源到目标前缀。
2. 每行把distance[0]设i，表示删完源前缀；先保存旧distance[0]给diagonal。
3. 逐列先保存above，再按字符相等或三种操作更新。
4. diagonal移到旧above，最终返回末项。

## 手工追踪

源"abc"目标"yabd"：可先在开头插入y得到yabc，再把c换成d，代价2。DP行（含0列）初始[0,1,2,3,4]，源a后[1,1,1,2,3]，ab后[2,2,2,1,2]，abc后[3,3,3,2,2]，最终2。

## 完整 C++17 实现

以下内容来自本课 [可编译头文件](../../../algorithms/cpp/solutions/p0072_edit_distance.hpp)，namespace用于让100题可同时进入同一工程。在线提交时一般只保留评测要求的类声明，并按平台要求处理标准库命名；本地辅助头文件负责统一导入标准库。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0072 {
class Solution {
public:
    int minDistance(std::string word1, std::string word2) {
        if (word2.size() > word1.size()) std::swap(word1, word2);
        std::vector<int> distance(word2.size() + 1);
        std::iota(distance.begin(), distance.end(), 0);
        for (std::size_t i = 1; i <= word1.size(); ++i) {
            int diagonal = distance[0]; distance[0] = static_cast<int>(i);
            for (std::size_t j = 1; j <= word2.size(); ++j) {
                int above = distance[j];
                if (word1[i-1] == word2[j-1]) distance[j] = diagonal;
                else distance[j] = 1 + std::min({above, distance[j-1], diagonal});
                diagonal = above;
            }
        }
        return distance.back();
    }
};
} // namespace hot100::p0072
```

## 复杂度核算

时间 O(mn)，算法辅助空间 O(min(m,n))；按值参数副本共 O(m+n)。返回标量，无递归。编辑距离对称，所以交换两串可优化列数。

## C++ 常见陷阱

- 每行开头的旧distance[0]必须先保存，不能把新i误当左上。
- 字符串长度转int要求可表示；实际超大文本的 O(mn) 也不可行。
- 操作成本若不对称，交换两串未必保持答案，需要重新设计接口与优化。

## 测试与运行

[完整测试源文件](../../../algorithms/cpp/tests/test_p0072.cpp)：穷举长度0到4的二字母字符串两两组合，与直接递归三操作基线对拍，并验证距离对称；含纯插入、替换与相同串。

从仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0072.cpp -o /tmp/hot100-cpp/test_p0072
/tmp/hot100-cpp/test_p0072
# 环境支持时增加运行期检查：
g++ -std=c++17 -Wall -Wextra -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0072.cpp -o /tmp/hot100-cpp/test_p0072_san
ASAN_OPTIONS=detect_leaks=0 /tmp/hot100-cpp/test_p0072_san
```

本次环境的LeakSanitizer受ptrace限制，因此命令显式关闭泄漏检测；AddressSanitizer与UndefinedBehaviorSanitizer仍启用。支持泄漏检测的本地环境可移除该环境变量。断言通过说明覆盖输入上的本地结果正确，不能替代证明、完整平台约束核对或在线评测。

## 面试追问

- 插入、删除、替换成本不同如何改递推与边界？
- 允许交换相邻字符的 Damerau-Levenshtein 距离需要什么额外历史？

## 练习与提示

- 练习1：输出操作序列。提示：使用完整二维表记录最后操作再逆向恢复。
- 练习2：若最大允许距离是k，怎样只计算主对角附近带状区域？提示：长度差>k可立即失败。
