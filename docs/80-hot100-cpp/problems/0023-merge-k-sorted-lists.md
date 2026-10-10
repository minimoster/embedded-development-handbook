# 23. 合并 K 个升序链表 · C++17

> 学习位置：链表 · 第 13 课。难度：HARD。这是一份独立编写的学习课，不是在线提交通过声明。

[官方题目](https://leetcode.cn/problems/merge-k-sorted-lists/) · 本课用简短契约与独立推演讲解，完整输入约束以官方题页为准。

## 1. 先说清输入、输出与修改边界

输入k条独立、无环、非降序链表，重连原节点生成一条排序链。允许空链与空数组；总节点数记为N。不同链的节点不能共享。

## 2. 从直觉到可用方案

每次从k个头里线性寻找最小值，需要O(Nk)时间。依次把新链并到越来越长的结果中，最坏也会多次扫描同一批节点。

各链只有头节点有资格成为下一个最小值，所以小根堆只放每条非空链的当前头。取出最小头后，仅把其后继补入堆，候选数从不超过k；这是多路归并的“每路维护一个前沿”。

## 3. 为什么正确：抓住不变量

堆在每轮开始恰含每条尚未耗尽链的首个未输出节点。由于每条链有序，任何深处节点都不小于该链头，因此堆顶是所有剩余节点的最小值。输出堆顶并把其后继入堆，恢复前沿不变量。每节点入堆、出堆各一次，所以结果完整且有序。

## 4. 动手过程

1. 定义比较器，让较大val具有较低优先级，建立局部priority_queue。
2. 把所有非空链头入堆。
3. 弹出node，先保存其next并将后继入堆。
4. 将node接到结果tail，继续直到堆空。
5. 末尾tail->next=nullptr，返回局部dummy.next。

## 5. 原创例子逐步追踪

三条链为[-2,4,9]、[0,3]、[-1,7]。初始候选-2、0、-1，弹出-2后补4；随后弹-1补7，弹0补3，弹3后第二条耗尽。后续4、7、9依次输出。堆里同时只保留三条链的前沿，不必把七个节点全部入堆。

## 6. 时间、栈与输出空间分别算

初始化逐个插入最多k个头，O(klog(k+1))；N次出入堆O(Nlog(k+1))，总O((N+k)log(k+1))，非空链数量h时可细化为O(k+Nlog(h+1))。辅助空间O(h)≤O(k)，无递归栈，无新输出节点。k=0、1时用log(k+1)避免不合适的零对数表述。

## 7. C++17实现与所有权

- priority_queue默认最大优先，因此比较器返回a->val>b->val才得到最小值堆顶。
- 不要用a->val-b->val做比较，整数差可能溢出。
- 比较器对同值返回false即可满足严格弱序；本实现不保证跨链同值稳定性。
- 堆放局部变量，避免同一Solution跨调用残留状态。
- 输入链共享节点会破坏“每节点只入一次”的前提，调用者必须遵守独立链契约。

下面是本仓库完整的头文件实现；support.hpp只提供标准库包含与题目节点声明，算法自身不依赖测试工具。在线评测通常已提供节点类型，提交时保留对应类并使用评测环境要求的作用域。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0023 {
class Solution {
    struct GreaterValue {
        bool operator()(const ListNode* a, const ListNode* b) const {
            return a->val > b->val;
        }
    };
public:
    ListNode* mergeKLists(std::vector<ListNode*>& lists) {
        std::priority_queue<ListNode*, std::vector<ListNode*>, GreaterValue> frontier;
        for (ListNode* head : lists) if (head) frontier.push(head);
        ListNode dummy;
        ListNode* tail = &dummy;
        while (!frontier.empty()) {
            ListNode* node = frontier.top();
            frontier.pop();
            if (node->next) frontier.push(node->next);
            tail->next = node;
            tail = node;
        }
        tail->next = nullptr;
        return dummy.next;
    }
};
} // namespace hot100::p0023
```

[查看可编译源文件](../../../algorithms/cpp/solutions/p0023_merge_k_sorted_lists.hpp) · [查看完整测试](../../../algorithms/cpp/tests/test_p0023.cpp)

## 实现复用与边界检查

每链一个候选的小根堆是有效方案。临时哑节点仍需回收，成员堆是否残留要检查所有退出路径。本实现使用局部堆使调用间独立性显式。

## 9. 本地验证

220组随机多路链，用全部值排序作参照；核对输出地址完整且唯一，另测空数组、空链与INT_MIN/INT_MAX。 测试采用固定种子，可复现每轮输入。断言针对返回值，也针对应保持的地址、连接或输入状态。内存由arena或对应destroy工具明确回收。本地测试通过仅说明这些可复现用例通过，不等于完整在线评测结论。

从仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp
c++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0023.cpp -o /tmp/hot100-cpp/test_p0023
/tmp/hot100-cpp/test_p0023
# 编译器支持时，再检查地址、未定义行为与泄漏
c++ -std=c++17 -Wall -Wextra -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0023.cpp -o /tmp/hot100-cpp/test_p0023_san
ASAN_OPTIONS=detect_leaks=1 /tmp/hot100-cpp/test_p0023_san
# 若执行环境在ptrace下明确拒绝LeakSanitizer，可仅运行地址/UB检查：
# ASAN_OPTIONS=detect_leaks=0 /tmp/hot100-cpp/test_p0023_san
```

## 10. 面试延伸

- 分治两两合并能否达到同阶复杂度？平衡合并层数O(logk)，每层所有节点处理一次。
- 数据源改为有序迭代器或磁盘文件时，为什么仍能使用同一个前沿模型？堆只需要各路当前值和推进能力。

## 11. 两道练习与提示

1. 实现分治合并，与堆版对随机多链比较值序列。提示：保持分治树平衡，不要从左向右累加合并。
2. 要求同值按链编号优先，如何扩展堆元素？提示：存{node,sourceIndex}并给比较器加入第二关键字。
