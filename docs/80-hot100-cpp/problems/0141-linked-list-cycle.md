# 141. 环形链表 · C++17

> 学习位置：链表 · 第 4 课。难度：EASY。这是一份独立编写的学习课，不是在线提交通过声明。

[官方题目](https://leetcode.cn/problems/linked-list-cycle/) · 本课用简短契约与独立推演讲解，完整输入约束以官方题页为准。

## 1. 先说清输入、输出与修改边界

判断从 head 连续追踪 next 是否会再次遇到同一节点。输入可能含环，不能修改 next，也不能把节点的值当作访问标记。

## 2. 从直觉到可用方案

记录访问过的地址可以直接发现重复，代价是 O(n) 集合空间。更省空间的办法来自跑道：一个指针每轮走一步，另一个走两步。若没有环，快指针先碰到空；若有环，两者进入环后相对距离每轮缩小一步，必定相遇。

“快慢”是移动规则，不能在初始时相等就宣布有环，必须先移动后比较。

## 3. 为什么正确：抓住不变量

无环时 next 路径有限，fast 或 fast->next 最终为空，循环退出。设环长 λ，在两者都进入环后，fast 比 slow 每轮多走一步，因此地址差模 λ 每轮改变 1，至多 λ 轮回到 0。相遇便证明存在重复访问的节点。

## 4. 动手过程

1. slow=fast=head。
2. 只有 fast 与 fast->next 非空时才继续。
3. slow 走一步，fast 走两步。
4. 移动后若地址相同返回 true；循环正常结束返回 false。

## 5. 原创例子逐步追踪

0→1→2→3，3 的 next 指向 1。移动后 slow/fast 的下标依次为 1/2、2/1、3/3，第三轮相遇。独立的两个值为 3 的节点并不会令地址相同。单节点自环则第一轮就相遇；单节点 next=nullptr 不进入循环。

## 6. 时间、栈与输出空间分别算

若无环节点数为 n，时间 O(n)；有环时设入环前节点数 μ、环长 λ，时间 O(μ+λ)。辅助空间 O(1)，没有递归或新增输出节点。

## 7. C++17实现与所有权

- && 短路求值保证先检查 fast，再访问 fast->next。
- 不能使用 listValues 打印有环输入，未经保护的遍历会无限循环。
- 测试 arena 按独立 unique_ptr 管理节点，析构不沿 next 递归，因此有环也安全。
- pos 是题目构造输入的描述，不是函数参数，算法只能依据指针结构。

下面是本仓库完整的头文件实现；support.hpp只提供标准库包含与题目节点声明，算法自身不依赖测试工具。在线评测通常已提供节点类型，提交时保留对应类并使用评测环境要求的作用域。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0141 {
class Solution {
public:
    bool hasCycle(ListNode* head) {
        ListNode* slow = head;
        ListNode* fast = head;
        while (fast && fast->next) {
            slow = slow->next;
            fast = fast->next->next;
            if (slow == fast) return true;
        }
        return false;
    }
};
} // namespace hot100::p0141
```

[查看可编译源文件](../../../algorithms/cpp/solutions/p0141_linked_list_cycle.hpp) · [查看完整测试](../../../algorithms/cpp/tests/test_p0141.cpp)

## 实现复用与边界检查

Floyd 检测需说明相遇的模环长证明、初始相等的陷阱与有环输入的回收方式；相遇不等于直接获得环入口。

## 9. 本地验证

长度1..30的无环链与每一种合法环入口，并与访问地址集合参照比较，含单节点自环。 测试采用固定种子，可复现每轮输入。断言针对返回值，也针对应保持的地址、连接或输入状态。内存由arena或对应destroy工具明确回收。本地测试通过仅说明这些可复现用例通过，不等于完整在线评测结论。

从仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp
c++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0141.cpp -o /tmp/hot100-cpp/test_p0141
/tmp/hot100-cpp/test_p0141
# 编译器支持时，再检查地址、未定义行为与泄漏
c++ -std=c++17 -Wall -Wextra -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0141.cpp -o /tmp/hot100-cpp/test_p0141_san
ASAN_OPTIONS=detect_leaks=1 /tmp/hot100-cpp/test_p0141_san
# 若执行环境在ptrace下明确拒绝LeakSanitizer，可仅运行地址/UB检查：
# ASAN_OPTIONS=detect_leaks=0 /tmp/hot100-cpp/test_p0141_san
```

## 10. 面试延伸

- 如果只使用一个游标但仍希望 O(1) 空间，有哪些代价？可以修改指针作标记，但违反本题不修改结构的设计。
- 若快指针改成每次三步，相遇之后能否直接套用入口定位？入口公式需要重新推导，不能机械照抄两倍速度。

## 11. 两道练习与提示

1. 对所有 μ∈[0,5]、λ∈[1,5] 构造环，检查算法。提示：环尾 next 接到第 μ 个节点。
2. 写集合版参照并比较。提示：集合保存 ListNode*，不要保存 val。
