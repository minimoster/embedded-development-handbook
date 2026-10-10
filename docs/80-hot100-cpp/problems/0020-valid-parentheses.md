# 20. 有效的括号：C++17 推导与实现

> 分类：栈 · 难度：EASY · 官方 Hot 100 顺序：69/100

## 题意与调用约定

输入只含三种左右括号，判断是否类型匹配且正确嵌套。空字符串在本实现中有效；若含其他字符，匹配过程会拒绝它。

题面与最新约束见 [力扣中文官方题目](https://leetcode.cn/problems/valid-parentheses/) / [LeetCode 官方英文题目](https://leetcode.com/problems/valid-parentheses/)。下文给出独立编写的推导、示例与实现；测试是本地验证，没有声称在线评测已通过。

## 从直觉到优化

反复删除相邻的匹配对直观但可能 O(n²)。从左向右处理时，一个右括号只能关闭最近尚未关闭的左括号。这种后进先出的关系自然是栈。栈中直接存“接下来期待的右括号”，能避免建立额外匹配表。

## 不变量与正确性

读完任意前缀后，栈由底到顶记录所有未闭合左括号对应的右括号；栈顶必须是下一个合法右括号。遇左括号入栈，遇右括号必须与顶相等后弹栈。任何失败前缀不可能靠后续字符补救；全部读完且栈空正好表示所有括号闭合。

## 算法步骤

1. 左括号转换成对应右括号后压栈。
2. 对右括号先检查栈非空，再检查类型。
3. 匹配后弹栈，保持剩余嵌套层次。
4. 扫描结束只在栈为空时返回 true。

## 手工追踪

输入 "{[()]}()"。读到 "{[(" 后栈为 [},],)]；遇 )、]、} 依次弹空；最后 () 再入栈并弹空。输入 "([)]" 在第三个字符 ) 处期待的是 ]，立刻失败。

## 完整 C++17 实现

以下内容来自本课 [可编译头文件](../../../algorithms/cpp/solutions/p0020_valid_parentheses.hpp)，namespace用于让100题可同时进入同一工程。在线提交时一般只保留评测要求的类声明，并按平台要求处理标准库命名；本地辅助头文件负责统一导入标准库。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0020 {
class Solution {
public:
    bool isValid(std::string s) {
        std::vector<char> expected;
        for (char c : s) {
            if (c == '(') expected.push_back(')');
            else if (c == '[') expected.push_back(']');
            else if (c == '{') expected.push_back('}');
            else {
                if (expected.empty() || expected.back() != c) return false;
                expected.pop_back();
            }
        }
        return expected.empty();
    }
};
} // namespace hot100::p0020
```

## 复杂度核算

时间 O(n)，vector 压栈为摊还 O(1)；额外空间 O(n)，最坏全部是左括号。无递归调用栈。

## C++ 常见陷阱

- 空 vector 上 back/pop_back 属于未定义行为，必须先判断 empty。
- 不应只统计左右括号数量，"([)]" 数量平衡却嵌套错误。
- 把字符串按值传入符合评测接口，但会持有一份 O(n) 参数副本；算法栈仍是 O(n)。

## 测试与运行

[完整测试源文件](../../../algorithms/cpp/tests/test_p0020.cpp)：穷举六种括号构成的长度0到6所有字符串，与反复删除匹配对的独立基线对拍；含错类型嵌套与未闭合。

从仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0020.cpp -o /tmp/hot100-cpp/test_p0020
/tmp/hot100-cpp/test_p0020
# 环境支持时增加运行期检查：
g++ -std=c++17 -Wall -Wextra -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0020.cpp -o /tmp/hot100-cpp/test_p0020_san
ASAN_OPTIONS=detect_leaks=0 /tmp/hot100-cpp/test_p0020_san
```

本次环境的LeakSanitizer受ptrace限制，因此命令显式关闭泄漏检测；AddressSanitizer与UndefinedBehaviorSanitizer仍启用。支持泄漏检测的本地环境可移除该环境变量。断言通过说明覆盖输入上的本地结果正确，不能替代证明、完整平台约束核对或在线评测。

## 面试追问

- 只有一种括号时能否只用计数器？为什么多种类型不行？
- 如何返回首次出错的位置或缺少的右括号序列？

## 练习与提示

- 练习1：把栈元素改为下标，返回错误位置。提示：保留左括号下标与期待类型。
- 练习2：验证“反复删除匹配对”与栈解等价。提示：用最内层配对与归纳法。
