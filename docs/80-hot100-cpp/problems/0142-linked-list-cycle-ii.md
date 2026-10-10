# 142. 环形链表 II · C++17

> 学习位置：链表 · 第 5 课。难度：MEDIUM。这是一份独立编写的学习课，不是在线提交通过声明。

[官方题目](https://leetcode.cn/problems/linked-list-cycle-ii/) · 本课用简短契约与独立推演讲解，完整输入约束以官方题页为准。

## 1. 先说清输入、输出与修改边界

返回链表的第一个入环节点地址；无环返回 nullptr。入口可能就是 head，也可能只有一个自环节点。函数不修改链表。

## 2. 从直觉到可用方案

集合版会把第一次重复遇到的地址当作入口，需要线性空间。Floyd 先找到环内相遇点，但相遇点通常不是入口。第二阶段把一个指针放回 head，另一个留在相遇点，两者每次各走一步，就会在入口相遇。

理解第二阶段需要追踪走过的距离，而不是只记“相遇后回头”口诀。

## 3. 为什么正确：抓住不变量

设头到入口距离为 μ，环长 λ，慢指针相遇时已走 t 步。快指针走 2t，因此 t 是 λ 的整数倍；相遇点在环上的位置是 (t-μ) mod λ。再从相遇点走 μ 步，其位置变为 t mod λ=0，正好是入口；从 head 走 μ 步也到入口。此前从 head 出发的指针还在环外，不可能提前与环内指针相等。

## 4. 动手过程

1. 用一倍/两倍速度移动，检查有无相遇。
2. 若快指针提前到空，返回 nullptr。
3. 相遇后令 entrance=head，保留 slow 在相遇点。
4. 两者同步走一步，直到地址相同，返回 entrance。

## 5. 原创例子逐步追踪

链为 A→B→C→D→E，E 接回 C，μ=2、λ=3。第一阶段第3轮 slow 在 D、fast 也在 D。第二阶段：A/D → B/E → C/C，返回 C。若入口为 A，相遇后 entrance 与 slow 可能立即相等，第二阶段零步也必须合法。

## 6. 时间、栈与输出空间分别算

时间 O(μ+λ)，无环则 O(n)；辅助空间 O(1)，无递归栈。返回输入节点的借用地址，没有新输出节点。

## 7. C++17实现与所有权

- 第二阶段必须两者同速；让 fast 继续两步会破坏距离证明。
- 节点值可以全相同，比较必须是地址。
- 不可在测试结束时沿环 delete 节点；使用 arena 独立拥有每个节点。
- 证明中“t 是环长整数倍”不等于“t 等于一圈”，快指针可能已经多绕圈。

下面是本仓库完整的头文件实现；support.hpp只提供标准库包含与题目节点声明，算法自身不依赖测试工具。在线评测通常已提供节点类型，提交时保留对应类并使用评测环境要求的作用域。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0142 {
class Solution {
public:
    ListNode* detectCycle(ListNode* head) {
        ListNode* slow = head;
        ListNode* fast = head;
        while (fast && fast->next) {
            slow = slow->next;
            fast = fast->next->next;
            if (slow == fast) {
                ListNode* entrance = head;
                while (entrance != slow) {
                    entrance = entrance->next;
                    slow = slow->next;
                }
                return entrance;
            }
        }
        return nullptr;
    }
};
} // namespace hot100::p0142
```

[查看可编译源文件](../../../algorithms/cpp/solutions/p0142_linked_list_cycle_ii.hpp) · [查看完整测试](../../../algorithms/cpp/tests/test_p0142.cpp)

## 实现复用与边界检查

相遇后从 head 与相遇点同步走，需要距离推导。入口恰为 head、单节点自环等边界应单独测试，不能直接返回相遇点。

## 9. 本地验证

长度1..35的每个环入口、无环与空链；返回值按入口地址核对，检查尾部连接未被修改。 测试采用固定种子，可复现每轮输入。断言针对返回值，也针对应保持的地址、连接或输入状态。内存由arena或对应destroy工具明确回收。本地测试通过仅说明这些可复现用例通过，不等于完整在线评测结论。

从仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp
c++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0142.cpp -o /tmp/hot100-cpp/test_p0142
/tmp/hot100-cpp/test_p0142
# 编译器支持时，再检查地址、未定义行为与泄漏
c++ -std=c++17 -Wall -Wextra -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0142.cpp -o /tmp/hot100-cpp/test_p0142_san
ASAN_OPTIONS=detect_leaks=1 /tmp/hot100-cpp/test_p0142_san
# 若执行环境在ptrace下明确拒绝LeakSanitizer，可仅运行地址/UB检查：
# ASAN_OPTIONS=detect_leaks=0 /tmp/hot100-cpp/test_p0142_san
```

## 10. 面试延伸

- 怎样求环长？从相遇点继续走，直到再次回到它，累计步数。
- 怎样求入环前链长？得到入口后，从 head 走到入口计数即可。

## 11. 两道练习与提示

1. 在已找到入口的基础上返回 {μ,λ}。提示：分别沿前缀及环独立计数。
2. 构造 μ 大于 λ 的输入并追踪第二阶段。提示：进入环后指针可以绕圈，公式仍按模 λ 成立。
