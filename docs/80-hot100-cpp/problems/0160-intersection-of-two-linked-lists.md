# 160. 相交链表 · C++17

> 学习位置：链表 · 第 1 课。难度：EASY。这是一份独立编写的学习课，不是在线提交通过声明。

[官方题目](https://leetcode.cn/problems/intersection-of-two-linked-lists/) · 本课用简短契约与独立推演讲解，完整输入约束以官方题页为准。

## 1. 先说清输入、输出与修改边界

两个无环单链表可能共用同一段尾链。返回第一个共用节点的地址；没有共用节点则返回 nullptr。值相同不构成相交，输入结构不允许被改动。

## 2. 从直觉到可用方案

把 A 的每个节点地址放进集合，再沿 B 寻找第一个命中点，思路直接，但额外空间与 A 的长度有关。真正的障碍不是查找速度，而是两个起点离公共尾链的距离不同。

设两条链长度为 m、n，让指针 a 走 A 后走 B，指针 b 走 B 后走 A。它们都补走了对方的独有前缀，路程差便被抵消。无需实际拼接链表，只在指针为空时切换头指针。

## 3. 为什么正确：抓住不变量

若存在公共尾段长度 c，独有前缀分别长 x=m-c、y=n-c。第一次进入公共尾段可能不同步；切换之后，a 经历 x+c+y，b 经历 y+c+x，同一步到达公共尾段入口。若不相交，两个指针最终在 nullptr 相等。循环比较的是地址，因此既不会把同值节点当成相交，也不会跨过第一个共同节点才停止。

## 4. 动手过程

1. 把两个游标分别放在 headA、headB。
2. 只要地址不同，各自推进一步；遇到 nullptr 时改走另一个链表的头。
3. 地址相等时返回该地址。nullptr 也是合法的共同终点。
4. 不保存长度，不修改任何 next。

## 5. 原创例子逐步追踪

A 为 8→3→C，B 为 5→6→1→C，公共段 C 为 9→4。两链独有前缀长度分别是 2、3。第一次走到 9 不同步；a 走完 A 后补上 B 的三节点前缀，b 走完 B 后补上 A 的两节点前缀，最终同时指向同一个 9。若把两条链上的 9 分别单独分配，即使所有值相同，最终答案也会是 nullptr。

## 6. 时间、栈与输出空间分别算

时间 O(m+n)，每个游标最多遍历两条链。辅助空间 O(1)，没有递归栈，也没有新分配的输出节点。返回值只是借用输入节点的一个地址。

## 7. C++17实现与所有权

- 必须写 a == b，不能写 a->val == b->val。
- 先处理空指针再读取 next；三目表达式在这里避免解引用空指针。
- 这套证明依赖无环前提。有环时可能无法终止，不能直接套用。
- 不拥有输入节点，不能 delete 返回节点；调用者管理两条链共享节点的生命周期。

下面是本仓库完整的头文件实现；support.hpp只提供标准库包含与题目节点声明，算法自身不依赖测试工具。在线评测通常已提供节点类型，提交时保留对应类并使用评测环境要求的作用域。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0160 {
class Solution {
public:
    ListNode* getIntersectionNode(ListNode* headA, ListNode* headB) {
        ListNode* a = headA;
        ListNode* b = headB;
        while (a != b) {
            a = a ? a->next : headB;
            b = b ? b->next : headA;
        }
        return a;
    }
};
} // namespace hot100::p0160
```

[查看可编译源文件](../../../algorithms/cpp/solutions/p0160_intersection_of_two_linked_lists.hpp) · [查看完整测试](../../../algorithms/cpp/tests/test_p0160.cpp)

## 实现复用与边界检查

长度对齐与换头法都在消除独有前缀的长度差，适用于无环链表。比较的是节点身份；不同值序列不能代替相交关系。

## 9. 本地验证

空链、独立同值单节点、头相同，以及49种独有前缀长度组合，共享尾节点按地址核对。 测试采用固定种子，可复现每轮输入。断言针对返回值，也针对应保持的地址、连接或输入状态。内存由arena或对应destroy工具明确回收。本地测试通过仅说明这些可复现用例通过，不等于完整在线评测结论。

从仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp
c++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0160.cpp -o /tmp/hot100-cpp/test_p0160
/tmp/hot100-cpp/test_p0160
# 编译器支持时，再检查地址、未定义行为与泄漏
c++ -std=c++17 -Wall -Wextra -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0160.cpp -o /tmp/hot100-cpp/test_p0160_san
ASAN_OPTIONS=detect_leaks=1 /tmp/hot100-cpp/test_p0160_san
# 若执行环境在ptrace下明确拒绝LeakSanitizer，可仅运行地址/UB检查：
# ASAN_OPTIONS=detect_leaks=0 /tmp/hot100-cpp/test_p0160_san
```

## 10. 面试延伸

- 如果有一个链表带环，哪些相交情形需要分开讨论？先分别求环入口，再讨论入口相同或不同。
- 为什么不能给链表 A 的尾部临时接上 B，再检测环？可以构造方法，但会修改共享结构，异常或并发读取时恢复更困难。

## 11. 两道练习与提示

1. 改写为长度对齐版本，再与换头版做随机交叉检查。提示：长链先走 abs(m-n) 步，之后同步前进。
2. 列出“两个头相同、一个头为空、两个独立单节点”的指针轨迹。提示：把 nullptr 也写成轨迹中的一个状态。
