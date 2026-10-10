# 21. 合并两个有序链表 · C++17

> 学习位置：链表 · 第 6 课。难度：EASY。这是一份独立编写的学习课，不是在线提交通过声明。

[官方题目](https://leetcode.cn/problems/merge-two-sorted-lists/) · 本课用简短契约与独立推演讲解，完整输入约束以官方题页为准。

## 1. 先说清输入、输出与修改边界

输入两条各自非降序、无环且节点不共用的链表。重连已有节点组成一条非降序链，返回新头；不新建数据节点。相同值时优先取第一条链，保持稳定。

## 2. 从直觉到可用方案

收集全部值、排序、重新建链，既用了额外内存，也浪费了输入已排序的信息。只需要比较两个当前头：较小者一定是所有尚未合并节点中的最小值。

哑节点把“第一次建立结果头”和“之后追加节点”统一成尾部接线操作。哑节点放在栈上，无需 new，也不会变成输出链的一部分。

## 3. 为什么正确：抓住不变量

每轮开始，dummy.next 到 tail 已经有序，并且恰好包含先前从两条链取出的节点；a、b 分别是未处理后缀的头。两条后缀内部有序，所以较小头是剩余全局最小节点。把它追加后不变量继续成立。当一条链为空时，另一条有序后缀可整体连接。

## 4. 动手过程

1. 创建局部 dummy 与 tail=&dummy。
2. 两条链都非空时比较头值，选择较小或相等时的 a。
3. 先让选中的链头前进，再把 tail 推进到追加节点。
4. 连接唯一剩下的后缀，返回 dummy.next。

## 5. 原创例子逐步追踪

A 为 1a→4a→4b，B 为 2b→4c。依次选择 1a、2b、4a、4b；A 为空后连接 4c。值顺序是 1,2,4,4,4，同值节点地址顺序体现了稳定选择策略。整个过程只改变原来的 next，没有复制节点。

## 6. 时间、栈与输出空间分别算

时间 O(m+n)，其中最后一段可一次接上但上界不变。辅助空间 O(1)，无递归栈，新增输出节点数为零。递归逐节点合并会产生 O(m+n) 最坏栈深。

## 7. C++17实现与所有权

- 局部 dummy 的地址不能返回；返回的是 dummy.next 指向的输入节点。
- 使用 <= 保留第一条链的同值顺序，有利于排序链表的稳定性。
- 输入共享尾链时普通合并可能把同一节点追加两次并成环；本题合并模型要求独立节点。
- 调用后旧的两个头不再代表两条互相独立的链，调用者不能各自沿 next 重复释放。

下面是本仓库完整的头文件实现；support.hpp只提供标准库包含与题目节点声明，算法自身不依赖测试工具。在线评测通常已提供节点类型，提交时保留对应类并使用评测环境要求的作用域。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0021 {
class Solution {
public:
    ListNode* mergeTwoLists(ListNode* a, ListNode* b) {
        ListNode dummy;
        ListNode* tail = &dummy;
        while (a && b) {
            if (a->val <= b->val) {
                tail->next = a;
                a = a->next;
            } else {
                tail->next = b;
                b = b->next;
            }
            tail = tail->next;
        }
        tail->next = a ? a : b;
        return dummy.next;
    }
};
} // namespace hot100::p0021
```

[查看可编译源文件](../../../algorithms/cpp/solutions/p0021_merge_two_sorted_lists.hpp) · [查看完整测试](../../../algorithms/cpp/tests/test_p0021.cpp)

## 实现复用与边界检查

递归合并需要计入线性调用栈。临时堆哑节点并不是输出的一部分，必须回收。本实现采用局部哑节点与迭代合并。

## 9. 本地验证

160组随机有序链与数组排序参照比较，含空链、负值、重复值；同值时核对稳定的地址选择。 测试采用固定种子，可复现每轮输入。断言针对返回值，也针对应保持的地址、连接或输入状态。内存由arena或对应destroy工具明确回收。本地测试通过仅说明这些可复现用例通过，不等于完整在线评测结论。

从仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp
c++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0021.cpp -o /tmp/hot100-cpp/test_p0021
/tmp/hot100-cpp/test_p0021
# 编译器支持时，再检查地址、未定义行为与泄漏
c++ -std=c++17 -Wall -Wextra -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0021.cpp -o /tmp/hot100-cpp/test_p0021_san
ASAN_OPTIONS=detect_leaks=1 /tmp/hot100-cpp/test_p0021_san
# 若执行环境在ptrace下明确拒绝LeakSanitizer，可仅运行地址/UB检查：
# ASAN_OPTIONS=detect_leaks=0 /tmp/hot100-cpp/test_p0021_san
```

## 10. 面试延伸

- 如何在不修改输入的前提下合并？创建新的结果节点，时间不变，输出空间 O(m+n)。
- 如果每条链采用降序，比较方向与尾部剩余连接该怎样调整？

## 11. 两道练习与提示

1. 写只读新建版，比较值结果并验证原 next 不变。提示：明确两个版本的所有权契约不同。
2. 用两条全为同值的链验证稳定性。提示：断言结果地址先来自第一条链，再来自第二条链。
