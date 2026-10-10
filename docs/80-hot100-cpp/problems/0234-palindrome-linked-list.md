# 234. 回文链表 · C++17

> 学习位置：链表 · 第 3 课。难度：EASY。这是一份独立编写的学习课，不是在线提交通过声明。

[官方题目](https://leetcode.cn/problems/palindrome-linked-list/) · 本课用简短契约与独立推演讲解，完整输入约束以官方题页为准。

## 1. 先说清输入、输出与修改边界

判断无环链表的值序列是否回文。本实现还保证返回前恢复原来的 next 结构，便于调用者继续使用同一条链。

## 2. 从直觉到可用方案

用数组保存全部值，再左右夹逼，需要 O(n) 空间。递归从尾部回程与头部指针比较，也隐藏了 O(n) 栈。若只把后半段反转，左右两半便能从各自的头同步比较。

快慢指针找到“前半段最后一个节点”：快指针每次走两步，慢指针走一步。奇数长度的中间值不用比较，偶数长度两半等长。比较完成后把后半段反转回来，避免一个只问真假值的接口悄悄改变输入。

## 3. 为什么正确：抓住不变量

slow 停在前半段末尾时，slow->next 开始的后半段长度为 floor(n/2)。反转后，从 head 与后半段新头出发，比较的第 i 对正好是原序列第 i 个与倒数第 i 个值。全部相等当且仅当回文。无论是否找到不等值，均执行恢复，所以结构后置条件也成立。

## 4. 动手过程

1. 长度小于 2 直接返回 true。
2. 在 fast->next 与 fast->next->next 都存在时同步推进，确定半段边界。
3. 反转 slow->next 保存的新头 second。
4. 用两个指针比較，遇到不等记录 false，但不要提前跳过恢复。
5. slow->next=reverse(second)，再返回比较结果。

## 5. 原创例子逐步追踪

7→4→2→4→7 中，slow 最终停在 2，后半段 4→7 被反为 7→4。比较 7/7、4/4 两对即足够，2 不参与。若链为 7→4→2→8→7，第二对 4/8 不同，仍恢复为原来的五个节点，之后才返回 false。

## 6. 时间、栈与输出空间分别算

找中点、两次半链反转与比较各为 O(n)，总时间 O(n)。辅助空间 O(1)，没有递归；结果是 bool，不新增输出节点。恢复使常数工作量增加，不改变阶数。

## 7. C++17实现与所有权

- 不能在检测到不等后直接 return false，否则输入仍被改成半链反转状态。
- 偶数链 1→2 的边界是第一个节点，需用 fast->next->next 判断。
- reverse 使用局部静态 helper，不保存跨调用的状态。
- 节点是借用的，不 delete；恢复操作同样只修改 next。

下面是本仓库完整的头文件实现；support.hpp只提供标准库包含与题目节点声明，算法自身不依赖测试工具。在线评测通常已提供节点类型，提交时保留对应类并使用评测环境要求的作用域。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0234 {
class Solution {
    static ListNode* reverse(ListNode* head) {
        ListNode* previous = nullptr;
        while (head) {
            ListNode* next = head->next;
            head->next = previous;
            previous = head;
            head = next;
        }
        return previous;
    }
public:
    bool isPalindrome(ListNode* head) {
        if (!head || !head->next) return true;
        ListNode* slow = head;
        ListNode* fast = head;
        while (fast->next && fast->next->next) {
            slow = slow->next;
            fast = fast->next->next;
        }
        ListNode* second = reverse(slow->next);
        bool equal = true;
        for (ListNode* a = head, *b = second; b; a = a->next, b = b->next) {
            if (a->val != b->val) { equal = false; break; }
        }
        slow->next = reverse(second);
        return equal;
    }
};
} // namespace hot100::p0234
```

[查看可编译源文件](../../../algorithms/cpp/solutions/p0234_palindrome_linked_list.hpp) · [查看完整测试](../../../algorithms/cpp/tests/test_p0234.cpp)

## 实现复用与边界检查

成员 flag 只在构造时初始化不够，连续调用前必须重置。递归回程法有 O(n) 栈；此实现局部化结果并测试先非回文再回文的顺序。

## 9. 本地验证

400条随机链用数组左右比较作参照，逐条验证函数返回后值与所有next恢复；同对象先非回文后回文。 测试采用固定种子，可复现每轮输入。断言针对返回值，也针对应保持的地址、连接或输入状态。内存由arena或对应destroy工具明确回收。本地测试通过仅说明这些可复现用例通过，不等于完整在线评测结论。

从仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp
c++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0234.cpp -o /tmp/hot100-cpp/test_p0234
/tmp/hot100-cpp/test_p0234
# 编译器支持时，再检查地址、未定义行为与泄漏
c++ -std=c++17 -Wall -Wextra -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0234.cpp -o /tmp/hot100-cpp/test_p0234_san
ASAN_OPTIONS=detect_leaks=1 /tmp/hot100-cpp/test_p0234_san
# 若执行环境在ptrace下明确拒绝LeakSanitizer，可仅运行地址/UB检查：
# ASAN_OPTIONS=detect_leaks=0 /tmp/hot100-cpp/test_p0234_san
```

## 10. 面试延伸

- 如果调用者明确允许破坏链表，能省掉什么？只能省恢复阶段，渐进复杂度不变。
- 如果链表在多线程中被读取，局部反转为什么仍不安全？恢复只保证函数返回后的状态，不保证中间状态不可见。

## 11. 两道练习与提示

1. 把数组版作为参照，对随机重复值链表比较结果。提示：std::equal 可比较正向与反向区间。
2. 用 6 个节点推演中点循环，说明两半为什么等长。提示：记录 fast 每次跨过的两条边。
