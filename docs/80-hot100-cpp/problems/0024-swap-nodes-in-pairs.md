# 24. 两两交换链表中的节点 · C++17

> 学习位置：链表 · 第 9 课。难度：MEDIUM。这是一份独立编写的学习课，不是在线提交通过声明。

[官方题目](https://leetcode.cn/problems/swap-nodes-in-pairs/) · 本课用简短契约与独立推演讲解，完整输入约束以官方题页为准。

## 1. 先说清输入、输出与修改边界

相邻两个节点为一组交换位置，最后不足一对的节点不动。必须重连节点，不能只交换 val。

## 2. 从直觉到可用方案

递归可先处理余下链，再把当前两节点反接，但栈深 O(n)。迭代更适合练习局部接线：始终保留当前待交换二元组的前驱 before，令 first、second 是组内两个节点。

交换后顺序为 before→second→first→after。先保存 first、second，再依次改三条边，可以避免把尚未处理后缀丢掉。

## 3. 为什么正确：抓住不变量

每轮开始，before 之前的部分已经两两交换且不会再次改动，before->next 是下一组起点。循环要求两节点都存在。三次接线把这一组从 first→second 改为 second→first，同时保留 after；before 更新为 first，恰好成为下一组前驱。退出时只剩零或一个节点，应原样保留。

## 4. 动手过程

1. 用局部 dummy 指向 head，before=&dummy。
2. 检查 before->next 与它的 next 都存在。
3. 保存 first、second，把 first 接到 second 原来的后继。
4. second 接到 first，before 接到 second。
5. before=first，继续处理下一对，返回 dummy.next。

## 5. 原创例子逐步追踪

链 3a→3b→6→8→9。第一组交换后为3b→3a→6→8→9，地址变化但相同值看不出差别。第二组变为3b→3a→8→6→9，before停在6；最后9没有搭档，保持原位。测试必须检查地址而不只是值。

## 6. 时间、栈与输出空间分别算

时间 O(n)，每轮处理两个节点；辅助空间 O(1)，无递归栈。复用输入节点，没有新输出分配。

## 7. C++17实现与所有权

- 循环用短路条件逐层检查，不能先访问不存在的第二节点。
- 每一对都应推进 before 到交换后的尾 first；若停在 second，会重复处理上一组。
- 用局部哑节点，不需要 delete。
- 只重连 next；val 相同的测试能揭露“交换值冒充换节点”的实现。

下面是本仓库完整的头文件实现；support.hpp只提供标准库包含与题目节点声明，算法自身不依赖测试工具。在线评测通常已提供节点类型，提交时保留对应类并使用评测环境要求的作用域。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0024 {
class Solution {
public:
    ListNode* swapPairs(ListNode* head) {
        ListNode dummy(0, head);
        ListNode* before = &dummy;
        while (before->next && before->next->next) {
            ListNode* first = before->next;
            ListNode* second = first->next;
            first->next = second->next;
            second->next = first;
            before->next = second;
            before = first;
        }
        return dummy.next;
    }
};
} // namespace hot100::p0024
```

[查看可编译源文件](../../../algorithms/cpp/solutions/p0024_swap_nodes_in_pairs.hpp) · [查看完整测试](../../../algorithms/cpp/tests/test_p0024.cpp)

## 实现复用与边界检查

迭代交换可以使用常数辅助空间。局部哑节点简化生命周期管理；测试应断言节点地址与连接顺序，而不只比较数值。

## 9. 本地验证

长度0..60全为同值的链，按地址验证两两交换；重复调用两次恢复原地址链。 测试采用固定种子，可复现每轮输入。断言针对返回值，也针对应保持的地址、连接或输入状态。内存由arena或对应destroy工具明确回收。本地测试通过仅说明这些可复现用例通过，不等于完整在线评测结论。

从仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp
c++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0024.cpp -o /tmp/hot100-cpp/test_p0024
/tmp/hot100-cpp/test_p0024
# 编译器支持时，再检查地址、未定义行为与泄漏
c++ -std=c++17 -Wall -Wextra -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0024.cpp -o /tmp/hot100-cpp/test_p0024_san
ASAN_OPTIONS=detect_leaks=1 /tmp/hot100-cpp/test_p0024_san
# 若执行环境在ptrace下明确拒绝LeakSanitizer，可仅运行地址/UB检查：
# ASAN_OPTIONS=detect_leaks=0 /tmp/hot100-cpp/test_p0024_san
```

## 10. 面试延伸

- 两两交换是 k=2 的分组反转，哪些边界在一般k时需要额外检查？需先证明完整组存在。
- 如果要求交换第i和第j个节点而非相邻节点，为什么要特别处理两者相邻的情况？两组前驱/后继会重叠。

## 11. 两道练习与提示

1. 输入四个值都为1的节点，列出期望地址顺序。提示：用a,b,c,d命名，结果应为b,a,d,c。
2. 连续调用swapPairs两次，验证结构复原。提示：奇数尾节点应在两次中都保持末尾。
