# 25. K 个一组翻转链表 · C++17

> 学习位置：链表 · 第 10 课。难度：HARD。这是一份独立编写的学习课，不是在线提交通过声明。

[官方题目](https://leetcode.cn/problems/reverse-nodes-in-k-group/) · 本课用简短契约与独立推演讲解，完整输入约束以官方题页为准。

## 1. 先说清输入、输出与修改边界

每完整的 k 个节点作为一组反转；末尾不足 k 个节点维持原顺序。重连节点，不交换值。官方 k 为正且不超过长度；本实现把 k≤1 视为不变操作。

## 2. 从直觉到可用方案

把节点地址存入数组后按组反排，需要 O(n) 辅助空间。递归按组处理也会留下调用栈。常数空间迭代的难点有两个：修改前确认组完整，反转时记住下一组的起点。

before 指向当前组前驱；先向后探测 k 步找到 kth。若找不到，不能动残组。若找到，after=kth->next 是反转的右边界，把 previous 初始化为 after，反转完成时组尾自然已经接好下一组。

## 3. 为什么正确：抓住不变量

每轮开始，before 之前是已经正确处理的完整组，后缀尚未改变。探测保证 [before->next,kth] 恰含 k 个节点。局部反转循环以 current==after 为结束，恰好反转这 k 条连接，并把原组头变成新组尾。before->next 指向 kth，再把 before 移到旧组头，维持组级不变量。不足组退出不修改，符合契约。

## 4. 动手过程

1. 局部 dummy.next=head；before=&dummy。
2. 从 before 探测 k 步，kth 为空则结束。
3. 保存 after=kth->next、oldHead=before->next。
4. 用 previous=after 反转，直到 current==after。
5. before->next=kth，before=oldHead，再处理下一组。

## 5. 原创例子逐步追踪

链 1→3→5→7→9→11→13，k=3。第一组kth=5、after=7，反成5→3→1→7。第二组kth=11、after=13，反成11→9→7→13。最后仅13，探测第2步就为空，停止。总结果5→3→1→11→9→7→13，最后残组从未被改动。

## 6. 时间、栈与输出空间分别算

时间 O(n)：每完整组先探测 k 个节点再反转 k 个节点，每节点被访问常数次；末尾探测少于 k 步。辅助空间 O(1)，无递归栈，无新增输出节点。

## 7. C++17实现与所有权

- 先反再判断够不够会把残组反错，恢复又增加分支；应先探测。
- 保存 after 后再写 next，避免失去未处理链。
- previous=after 可以省去反完再给尾接后缀的步骤，但必须用 current!=after 为界。
- 只有节点地址和连接变化；k=1、空链、长度刚好等于k应覆盖。

下面是本仓库完整的头文件实现；support.hpp只提供标准库包含与题目节点声明，算法自身不依赖测试工具。在线评测通常已提供节点类型，提交时保留对应类并使用评测环境要求的作用域。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0025 {
class Solution {
public:
    ListNode* reverseKGroup(ListNode* head, int k) {
        if (k <= 1) return head;
        ListNode dummy(0, head);
        ListNode* before = &dummy;
        while (true) {
            ListNode* kth = before;
            for (int i = 0; i < k && kth; ++i) kth = kth->next;
            if (!kth) break;
            ListNode* after = kth->next;
            ListNode* oldHead = before->next;
            ListNode* previous = after;
            ListNode* current = oldHead;
            while (current != after) {
                ListNode* next = current->next;
                current->next = previous;
                previous = current;
                current = next;
            }
            before->next = kth;
            before = oldHead;
        }
        return dummy.next;
    }
};
} // namespace hot100::p0025
```

[查看可编译源文件](../../../algorithms/cpp/solutions/p0025_reverse_nodes_in_k_group.hpp) · [查看完整测试](../../../algorithms/cpp/tests/test_p0025.cpp)

## 实现复用与边界检查

组内递归反转会产生 O(k) 调用栈，不能只数指针变量就宣称 O(1) 辅助空间。此实现组内和组间都迭代，并使用局部哑节点。

## 9. 本地验证

300组随机长度与k，用地址数组的完整分块reverse作参照，检验不足组不动且没有重复或遗漏地址。 测试采用固定种子，可复现每轮输入。断言针对返回值，也针对应保持的地址、连接或输入状态。内存由arena或对应destroy工具明确回收。本地测试通过仅说明这些可复现用例通过，不等于完整在线评测结论。

从仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp
c++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0025.cpp -o /tmp/hot100-cpp/test_p0025
/tmp/hot100-cpp/test_p0025
# 编译器支持时，再检查地址、未定义行为与泄漏
c++ -std=c++17 -Wall -Wextra -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0025.cpp -o /tmp/hot100-cpp/test_p0025_san
ASAN_OPTIONS=detect_leaks=1 /tmp/hot100-cpp/test_p0025_san
# 若执行环境在ptrace下明确拒绝LeakSanitizer，可仅运行地址/UB检查：
# ASAN_OPTIONS=detect_leaks=0 /tmp/hot100-cpp/test_p0025_san
```

## 10. 面试延伸

- 若末尾不足k也要反转，只需怎样改变组探测？找到真实尾作为kth并允许不足组，循环边界仍可复用。
- 若每组大小按数组变化，before/after不变量哪些部分仍成立？每轮探测当前组长度即可。

## 11. 两道练习与提示

1. 用数组地址反排每个完整分块，作为随机差分参照。提示：最后长度不足k的分块不调用reverse。
2. 将实现抽成反转半开区间[first,after)的helper。提示：helper需同时提供新头与新尾，否则外层还要重新扫描。
