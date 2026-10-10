# 206. 反转链表 · C++17

> 学习位置：链表 · 第 2 课。难度：EASY。这是一份独立编写的学习课，不是在线提交通过声明。

[官方题目](https://leetcode.cn/problems/reverse-linked-list/) · 本课用简短契约与独立推演讲解，完整输入约束以官方题页为准。

## 1. 先说清输入、输出与修改边界

把无环单链表的 next 方向反过来，并返回新头。节点值、节点地址集合保持不变；空链表仍为空。

## 2. 从直觉到可用方案

先把值放进数组再倒着创建链表，能得到相同值序列，却丢失“原地重连原节点”的训练目标。递归可在回程时反转，但每个节点都留下一个栈帧。

迭代把链表看成两部分：已经反好的前缀和未处理的后缀。每次只从后缀摘下一个节点，放到前缀的最前面。关键不是“赋值几次”，而是修改 next 前必须保存原后继，否则剩下的链会失联。

## 3. 为什么正确：抓住不变量

循环开始时，previous 指向已经反转的前缀，current 指向仍保持原方向的后缀，两部分覆盖全部原节点且互不重叠。保存 current->next 后把 current 接到 previous 前面，再移动两游标，保持该不变量。后缀为空时，previous 包含所有节点，顺序恰是原顺序的逆序。

## 4. 动手过程

1. previous=nullptr，current=head。
2. 保存 following=current->next。
3. current->next=previous，完成一条边的反转。
4. previous=current，current=following，直到 current 为空。
5. 返回 previous；原 head 此时是尾节点，其 next 已在首轮置空。

## 5. 原创例子逐步追踪

原链 6→2→9：第一轮得到 6→null，剩余 2→9；第二轮得到 2→6→null，剩余 9；第三轮得到 9→2→6→null，剩余为空。观察 6 的 next 只需要在第一次处理时改为 nullptr，无须再专门寻找尾节点。

## 6. 时间、栈与输出空间分别算

时间 O(n)，每个节点处理一次；辅助空间 O(1)，递归栈 O(1)。输出复用原节点，新增输出空间为零。递归版本虽短，栈空间仍为 O(n)，C++ 不保证尾递归优化。

## 7. C++17实现与所有权

- 保存后继必须在覆盖 next 之前，执行顺序是本题的核心。
- 输入必须无环，否则“处理到空”不成立。
- 不分配、不释放节点；arena 所有权与指针重连互不冲突。
- 反转两次应恢复相同的地址链，测试不能只看值而忽略丢节点或成环。

下面是本仓库完整的头文件实现；support.hpp只提供标准库包含与题目节点声明，算法自身不依赖测试工具。在线评测通常已提供节点类型，提交时保留对应类并使用评测环境要求的作用域。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0206 {
class Solution {
public:
    ListNode* reverseList(ListNode* head) {
        ListNode* previous = nullptr;
        for (ListNode* current = head; current;) {
            ListNode* following = current->next;
            current->next = previous;
            previous = current;
            current = following;
        }
        return previous;
    }
};
} // namespace hot100::p0206
```

[查看可编译源文件](../../../algorithms/cpp/solutions/p0206_reverse_linked_list.hpp) · [查看完整测试](../../../algorithms/cpp/tests/test_p0206.cpp)

## 实现复用与边界检查

递归反转需要 O(n) 调用栈。此实现使用迭代，避免把只声明几个指针误当成整体 O(1) 空间。

## 9. 本地验证

300条固定种子随机链，核对反向值与反向地址；再反转一次逐边验证原拓扑恢复。 测试采用固定种子，可复现每轮输入。断言针对返回值，也针对应保持的地址、连接或输入状态。内存由arena或对应destroy工具明确回收。本地测试通过仅说明这些可复现用例通过，不等于完整在线评测结论。

从仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp
c++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0206.cpp -o /tmp/hot100-cpp/test_p0206
/tmp/hot100-cpp/test_p0206
# 编译器支持时，再检查地址、未定义行为与泄漏
c++ -std=c++17 -Wall -Wextra -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0206.cpp -o /tmp/hot100-cpp/test_p0206_san
ASAN_OPTIONS=detect_leaks=1 /tmp/hot100-cpp/test_p0206_san
# 若执行环境在ptrace下明确拒绝LeakSanitizer，可仅运行地址/UB检查：
# ASAN_OPTIONS=detect_leaks=0 /tmp/hot100-cpp/test_p0206_san
```

## 10. 面试延伸

- 如何反转一个闭区间 [left,right]？保留区间前驱与区间后继，再复用局部反转。
- 如何处理循环链表？需要明确终止边界，不能继续使用 nullptr 作为结束标记。

## 11. 两道练习与提示

1. 不运行代码，写出 4→8→1 的每轮 previous/current/following。提示：每轮画两个不相连的链段。
2. 实现反转后再反转，并检查每个地址恢复。提示：记录原节点地址数组，值重复时地址检查更有意义。
