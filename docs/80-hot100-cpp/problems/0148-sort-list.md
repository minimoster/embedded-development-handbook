# 148. 排序链表 · C++17

> 学习位置：链表 · 第 12 课。难度：MEDIUM。这是一份独立编写的学习课，不是在线提交通过声明。

[官方题目](https://leetcode.cn/problems/sort-list/) · 本课用简短契约与独立推演讲解，完整输入约束以官方题页为准。

## 1. 先说清输入、输出与修改边界

把无环单链表按非降序排序，重连已有节点。本实现采用稳定的自底向上归并，要求时间O(nlogn)、辅助空间O(1)，不用递归栈。

## 2. 从直觉到可用方案

用堆保存全部节点可排序，却需要O(n)堆空间。自顶向下归并较自然，但递归拆分用O(logn)栈；如果合并本身也是递归，最顶层合并甚至可能产生O(n)栈。

链表适合归并，因为合并只要移动连接，不需要数组的额外缓冲区。自底向上从长度1的有序段开始，两两合并，段长依次为1、2、4、8……，直到覆盖整链。

## 3. 为什么正确：抓住不变量

每轮段长为width，轮开始时链表可划分为长度至多width且内部有序的连续段。每次切出左右两段并稳定合并，得到长度至多2width的有序段。所有段依次接回后，下一轮不变量成立。width达到n时只有一个有序段，且每轮仅重新链接原节点，不丢失也不重复节点。

## 4. 动手过程

1. 遍历计算节点数count，空/单节点直接返回。
2. width从1递增，局部dummy指向当前头。
3. split从current切出左段，再切出右段，保存下一组current。
4. 迭代merge，把两段接到tail；接完后tail必须指向新段末尾。
5. 每轮结束更新head=dummy.next，将width加倍；接近count时直接设为count避免倍增溢出。

## 5. 原创例子逐步追踪

原链5→1→4→2→3。width=1合并(5,1)、(4,2)、(3,空)，得到1→5→2→4→3。width=2合并(1→5,2→4)，得到1→2→4→5→3。width=4合并前四个与最后3，得到1→2→3→4→5。每轮切断是为了让merge明确只处理当前两段。

## 6. 时间、栈与输出空间分别算

计数O(n)，归并轮数ceil(log₂n)，每轮切段、合并、寻找段尾的总工作均O(n)，故总时间O(nlogn)。辅助空间O(1)，调用栈O(1)，没有新增输出节点。测试中额外的地址数组属于验证程序，不属于算法空间。

## 7. C++17实现与所有权

- split需要把段尾next置空，否则merge会误吞后面的组。
- 同值优先左段，稳定性才能跨每轮归并传递。
- width和count用size_t；width>count/2时直接到count，避免width*=2溢出。
- merge完成后走到tail，虽多扫描一次但每轮总量仍O(n)，不是O(n²)。
- 不释放任何输入节点，arena仍按原有unique_ptr持有它们。

下面是本仓库完整的头文件实现；support.hpp只提供标准库包含与题目节点声明，算法自身不依赖测试工具。在线评测通常已提供节点类型，提交时保留对应类并使用评测环境要求的作用域。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0148 {
class Solution {
    static ListNode* split(ListNode* head, std::size_t width) {
        if (!head) return nullptr;
        for (std::size_t i = 1; i < width && head->next; ++i) head = head->next;
        ListNode* rest = head->next;
        head->next = nullptr;
        return rest;
    }
    static ListNode* appendMerge(ListNode* a, ListNode* b, ListNode* tail) {
        while (a && b) {
            if (a->val <= b->val) { tail->next = a; a = a->next; }
            else { tail->next = b; b = b->next; }
            tail = tail->next;
        }
        tail->next = a ? a : b;
        while (tail->next) tail = tail->next;
        return tail;
    }
public:
    ListNode* sortList(ListNode* head) {
        std::size_t count = 0;
        for (ListNode* p = head; p; p = p->next) ++count;
        if (count < 2) return head;
        for (std::size_t width = 1; width < count;
             width = width > count / 2 ? count : width * 2) {
            ListNode dummy(0, head);
            ListNode* tail = &dummy;
            ListNode* current = head;
            while (current) {
                ListNode* left = current;
                ListNode* right = split(left, width);
                current = split(right, width);
                tail = appendMerge(left, right, tail);
            }
            head = dummy.next;
        }
        return head;
    }
};
} // namespace hot100::p0148
```

[查看可编译源文件](../../../algorithms/cpp/solutions/p0148_sort_list.hpp) · [查看完整测试](../../../algorithms/cpp/tests/test_p0148.cpp)

## 实现复用与边界检查

递归逐节点合并的最坏调用栈为 O(n)，无法满足常数辅助空间目标。此实现自底向上拆段并迭代合并，避免递归栈。

## 9. 本地验证

400组随机含重复值链，std::stable_sort地址序列作参照；另有3000节点逆序链检测多轮合并。 测试采用固定种子，可复现每轮输入。断言针对返回值，也针对应保持的地址、连接或输入状态。内存由arena或对应destroy工具明确回收。本地测试通过仅说明这些可复现用例通过，不等于完整在线评测结论。

从仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp
c++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0148.cpp -o /tmp/hot100-cpp/test_p0148
/tmp/hot100-cpp/test_p0148
# 编译器支持时，再检查地址、未定义行为与泄漏
c++ -std=c++17 -Wall -Wextra -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0148.cpp -o /tmp/hot100-cpp/test_p0148_san
ASAN_OPTIONS=detect_leaks=1 /tmp/hot100-cpp/test_p0148_san
# 若执行环境在ptrace下明确拒绝LeakSanitizer，可仅运行地址/UB检查：
# ASAN_OPTIONS=detect_leaks=0 /tmp/hot100-cpp/test_p0148_san
```

## 10. 面试延伸

- 如果只要求O(nlogn)时间、不要求常数空间，自顶向下版为什么更易读？中点拆分直接对应递归定义。
- 如何验证稳定排序？相同val节点的最终地址顺序应与原顺序一致。
- 链表快速排序为何不一定是好选择？分区接线复杂，坏轴选择仍会退化。

## 11. 两道练习与提示

1. 给每个节点附加原始序号，仅按val排序，检查相同val的序号顺序。提示：本地用地址→序号映射即可。
2. 让split返回{段头,剩余头}并重写主循环。提示：必须仍能找到当前段尾来断开，返回值设计应服务接线不变量。
