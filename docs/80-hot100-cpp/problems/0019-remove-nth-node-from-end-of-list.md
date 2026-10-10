# 19. 删除链表的倒数第 N 个结点 · C++17

> 学习位置：链表 · 第 8 课。难度：MEDIUM。这是一份独立编写的学习课，不是在线提交通过声明。

[官方题目](https://leetcode.cn/problems/remove-nth-node-from-end-of-list/) · 本课用简短契约与独立推演讲解，完整输入约束以官方题页为准。

## 1. 先说清输入、输出与修改边界

从无环链表中摘掉倒数第 n 个节点，返回剩余链的头。官方保证 1≤n≤长度。本仓库节点由调用者/arena 拥有，所以“删除”表示移出链，不在算法内 delete 节点；还把被摘节点的 next 清空。

## 2. 从直觉到可用方案

先数长度 L，再走到第 L-n 个节点的前驱，需要两次遍历。快慢指针把倒数转换为固定距离：从哑节点出发，让 fast 先走 n 步，再同步推进。fast 到最后一个节点时，slow 正好在待删节点前面。

用哑节点是为了让删除 head 与删除中间节点走相同的接线语句。

## 3. 为什么正确：抓住不变量

fast 先走 n 步后，两游标之间始终相隔 n 条 next 边。同步推进不改变间距。fast 到最后节点时，其后还有0个节点，因此 slow->next 后面恰有 n-1 个节点，slow->next 就是倒数第 n 个。修改一条前驱边即可保留其他节点顺序。

## 4. 动手过程

1. 若 n≤0 返回原 head；局部 dummy.next=head。
2. fast 从 dummy 前进 n 次；若长度不足，防御性地返回原链。
3. 只要 fast->next 存在，fast、slow 同步走。
4. victim=slow->next，把 slow->next 接到 victim->next。
5. victim->next=nullptr，返回 dummy.next。

## 5. 原创例子逐步追踪

链 2→5→7→8，n=3。fast 从 dummy 走3步到7，slow还在dummy。同步走一次后 fast=8、slow=2，于是摘掉slow->next，也就是5。n=4时无需同步推进，slow仍是dummy，直接摘掉头2。

## 6. 时间、栈与输出空间分别算

时间 O(L)，虽然含预先推进和同步推进，但 fast 总共只走到尾。辅助空间 O(1)，无递归或新输出节点。被摘节点仍属于调用者，其回收不属于链形调整的辅助空间。

## 7. C++17实现与所有权

- 题目中的“删除”不自动授予对借用指针调用 delete 的权利，要先明确生命周期契约。
- 若调用者用 raw new 独立拥有各节点，可在获取被摘节点后自行 delete；arena 输入不能再 delete。
- slow 必须落在前驱，不能只找到目标本身；单链表没有 prev。
- 本实现对非法 n 保持原链，属于额外防御，不是官方题目要求。

下面是本仓库完整的头文件实现；support.hpp只提供标准库包含与题目节点声明，算法自身不依赖测试工具。在线评测通常已提供节点类型，提交时保留对应类并使用评测环境要求的作用域。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0019 {
class Solution {
public:
    ListNode* removeNthFromEnd(ListNode* head, int n) {
        if (n <= 0) return head;
        ListNode dummy(0, head);
        ListNode* fast = &dummy;
        for (int i = 0; i < n; ++i) {
            fast = fast->next;
            if (!fast) return head;
        }
        ListNode* slow = &dummy;
        while (fast->next) { fast = fast->next; slow = slow->next; }
        ListNode* victim = slow->next;
        slow->next = victim->next;
        victim->next = nullptr; // borrowed node: the caller retains ownership
        return dummy.next;
    }
};
} // namespace hot100::p0019
```

[查看可编译源文件](../../../algorithms/cpp/solutions/p0019_remove_nth_node_from_end_of_list.hpp) · [查看完整测试](../../../algorithms/cpp/tests/test_p0019.cpp)

## 实现复用与边界检查

递归计数若存为成员，每次入口都必须重置；临时哑节点也要回收。只有拥有节点的算法才能 delete 被摘节点。本实现借用输入，避免与测试 arena 的 unique_ptr 双重释放。

## 9. 本地验证

长度1..25下每个合法倒数位置，共325种删除位置，用数组擦除作参照；被摘节点断开但仍由arena回收。 测试采用固定种子，可复现每轮输入。断言针对返回值，也针对应保持的地址、连接或输入状态。内存由arena或对应destroy工具明确回收。本地测试通过仅说明这些可复现用例通过，不等于完整在线评测结论。

从仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp
c++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0019.cpp -o /tmp/hot100-cpp/test_p0019
/tmp/hot100-cpp/test_p0019
# 编译器支持时，再检查地址、未定义行为与泄漏
c++ -std=c++17 -Wall -Wextra -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0019.cpp -o /tmp/hot100-cpp/test_p0019_san
ASAN_OPTIONS=detect_leaks=1 /tmp/hot100-cpp/test_p0019_san
# 若执行环境在ptrace下明确拒绝LeakSanitizer，可仅运行地址/UB检查：
# ASAN_OPTIONS=detect_leaks=0 /tmp/hot100-cpp/test_p0019_san
```

## 10. 面试延伸

- 接口怎样同时返回新头与被摘节点？可用 pair<ListNode*,ListNode*> 明确把释放决定交给调用者。
- 如果链表有环，倒数第 n 个是否有定义？需要重新约定链长度，不能直接使用本题算法。

## 11. 两道练习与提示

1. 实现返回被摘节点的扩展接口。提示：先保存 victim，再断开 victim->next。
2. 对每个长度L=1..8、每个合法n验证结果。提示：数组参照删除下标 L-n。
