# 394. 字符串解码：C++17 推导与实现

> 分类：栈 · 难度：MEDIUM · 官方 Hot 100 顺序：71/100

## 题意与调用约定

有效编码由普通字母与 k[子表达式] 组成，k为正整数且可多位、可嵌套，返回展开字符串。输入合法是前提，不将本函数作为不可信编码的完整解析器。

题面与最新约束见 [力扣中文官方题目](https://leetcode.cn/problems/decode-string/) / [LeetCode 官方英文题目](https://leetcode.com/problems/decode-string/)。下文给出独立编写的推导、示例与实现；测试是本地验证，没有声称在线评测已通过。

## 从直觉到优化

只替换最内层括号可以完成工作，但反复搜索括号容易重复扫描。读到左括号时，当前层的前缀与重复次数需要暂存；读到右括号时，再把完整子串复制指定次数并接回前缀。显式 frame 栈把递归调用现场搬到容器中。

## 不变量与正确性

current 始终是当前嵌套层已读内容的完整解码；每个 frame 保存上一层在左括号前的解码前缀和该子块次数。闭括号把 current 完整重复后接回 frame.prefix，因此新 current 又满足上一层语义。有效输入保证右括号总有对应 frame。

## 算法步骤

1. 数字累积 count=count*10+digit。
2. 遇左括号，移动保存前缀与次数，重置当前层。
3. 普通字符追加到 current。
4. 遇右括号，弹出 frame，预留所需空间，追加 repeat 次子串，再移动回 current。

## 手工追踪

编码 "2[x3[y]]z"：外层 frame("",2)，current读到x；内层保存 frame("x",3)，current变y；内层闭合得到xyyy；外层闭合得到xyyyxyyy；最后追加z。结果长度9。

## 完整 C++17 实现

以下内容来自本课 [可编译头文件](../../../algorithms/cpp/solutions/p0394_decode_string.hpp)，namespace用于让100题可同时进入同一工程。在线提交时一般只保留评测要求的类声明，并按平台要求处理标准库命名；本地辅助头文件负责统一导入标准库。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0394 {
class Solution {
public:
    std::string decodeString(std::string s) {
        struct Frame { std::string prefix; std::size_t repeat; };
        std::vector<Frame> frames;
        std::string current;
        std::size_t count = 0;
        for (char c : s) {
            if (c >= '0' && c <= '9') count = count * 10 + static_cast<std::size_t>(c - '0');
            else if (c == '[') {
                frames.push_back({std::move(current), count});
                current.clear(); count = 0;
            } else if (c == ']') {
                Frame f = std::move(frames.back()); frames.pop_back();
                if (!current.empty() && f.repeat > (f.prefix.max_size() - f.prefix.size()) / current.size())
                    throw std::length_error("decoded string too large");
                f.prefix.reserve(f.prefix.size() + f.repeat * current.size());
                for (std::size_t i = 0; i < f.repeat; ++i) f.prefix += current;
                current = std::move(f.prefix);
            } else current.push_back(c);
        }
        return current;
    }
};
} // namespace hot100::p0394
```

## 复杂度核算

设编码长度 N、最终解码长度 L、最大嵌套深度 d，展开过程中复制字符总量 E。时间 O(N+E)，最坏 O(N+dL)，不是一概 O(N+L)：1[1[...长串...]] 也会逐层复制。峰值额外内存 O(N+L)，frame元数据 O(d)，输出占 O(L)；无递归调用栈。

## C++ 常见陷阱

- 次数累积采用 size_t，但超大数字仍可能溢出；官方有输出大小约束，不承诺任意超大不可信输入。
- reserve 前用除法检验乘积，避免 repeat*长度先溢出再检查。
- std::move 后显式 clear，别依赖 moved-from 字符串为空；移动后只保证有效状态。
- frames.back() 的安全依赖有效语法；若做生产解析器，应补语法错误、深度及输出配额检查。

## 测试与运行

[完整测试源文件](../../../algorithms/cpp/tests/test_p0394.cpp)：300组固定种子语法生成器构造深度不超过3的嵌套编码，与递归下降解析器对拍；含多位次数、普通文本与空串。

从仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0394.cpp -o /tmp/hot100-cpp/test_p0394
/tmp/hot100-cpp/test_p0394
# 环境支持时增加运行期检查：
g++ -std=c++17 -Wall -Wextra -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0394.cpp -o /tmp/hot100-cpp/test_p0394_san
ASAN_OPTIONS=detect_leaks=0 /tmp/hot100-cpp/test_p0394_san
```

本次环境的LeakSanitizer受ptrace限制，因此命令显式关闭泄漏检测；AddressSanitizer与UndefinedBehaviorSanitizer仍启用。支持泄漏检测的本地环境可移除该环境变量。断言通过说明覆盖输入上的本地结果正确，不能替代证明、完整平台约束核对或在线评测。

## 面试追问

- 如何建立语法树后一次生成输出，减少多层重复复制？
- 如何限制输出大小，防止很短的压缩输入造成内存爆炸？

## 练习与提示

- 练习1：加入语法验证与最大输出长度。提示：右括号前确认 frame 非空，次数累积前检查上限。
- 练习2：实现递归下降参考解析器并随机生成有效编码对拍。提示：生成器同时控制深度和每层片段数。
