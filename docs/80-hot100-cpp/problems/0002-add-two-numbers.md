# 2. 两数相加 · C++17

> 学习位置：链表 · 第 7 课。难度：MEDIUM。这是一份独立编写的学习课，不是在线提交通过声明。

[官方题目](https://leetcode.cn/problems/add-two-numbers/) · 本课用简短契约与独立推演讲解，完整输入约束以官方题页为准。

## 1. 先说清输入、输出与修改边界

两条链按低位在前存放非负整数，每节点是 0..9 的一位数字。返回独立分配的新链表示和，输入链保持不变。官方输入非空；本实现也容许一个输入为空。

## 2. 从直觉到可用方案

把链表转换为整数再相加，长度稍大就溢出；即使用 long long 也只是推迟失败。小学竖式每次只处理一位与上一位进位，不需要知道完整整数。

逆序存储恰好让头节点是个位。每轮把两边当前位与 carry 加起来，结果位为 total%10，新进位为 total/10。即使两条链都结束，最后的 carry 仍可能要求创建一个节点。

## 3. 为什么正确：抓住不变量

处理了 i 位以后，结果前 i 个节点准确表示和的低 i 位，carry 保存低 i 位相加产生的向第 i 位进位。每轮 total=a_i+b_i+carry∈[0,19]，分解为个位和进位恰好维持竖式关系。循环包含 carry 条件，所以最高位进位也不会丢失。

## 4. 动手过程

1. 局部 dummy、tail 和 carry=0。
2. 只要任一链未完或 carry 非零，计算这一位总数。
3. 把非空输入指针各推进一步。
4. 新建 total%10 的节点，carry=total/10。
5. 返回 dummy.next；调用者最终负责销毁全部新节点。

## 5. 原创例子逐步追踪

低位链 A=[8,9,9]，B=[7,0,2]，即 998+207。个位：8+7=15，写5进1；十位：9+0+1=10，写0进1；百位：9+2+1=12，写2进1；输入完后再写1。结果 [5,0,2,1] 表示1205。这里每一步都没有存储998或1205这个完整整数。

## 6. 时间、栈与输出空间分别算

时间 O(max(m,n))，结果长度最多 max(m,n)+1。除输出外辅助空间 O(1)，无递归栈；输出节点空间 O(max(m,n))，必须与辅助空间分开报告。

## 7. C++17实现与所有权

- 数字范围保证 total≤19，int 足够，不需要大整数类型。
- while(l1||l2||carry) 比结束后单独补节点更统一。
- 哑节点在栈上，实际结果节点用 new；输入节点不可 delete。
- 这份教学实现按正常内存分配成功的环境运行；若产品代码要求分配失败时强异常保证，可暂存 unique_ptr 节点后统一交出所有权。

下面是本仓库完整的头文件实现；support.hpp只提供标准库包含与题目节点声明，算法自身不依赖测试工具。在线评测通常已提供节点类型，提交时保留对应类并使用评测环境要求的作用域。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0002 {
class Solution {
public:
    ListNode* addTwoNumbers(ListNode* a, ListNode* b) {
        ListNode dummy;
        ListNode* tail = &dummy;
        int carry = 0;
        while (a || b || carry) {
            int total = carry;
            if (a) { total += a->val; a = a->next; }
            if (b) { total += b->val; b = b->next; }
            tail->next = new ListNode(total % 10);
            tail = tail->next;
            carry = total / 10;
        }
        return dummy.next;
    }
};
} // namespace hot100::p0002
```

[查看可编译源文件](../../../algorithms/cpp/solutions/p0002_add_two_numbers.hpp) · [查看完整测试](../../../algorithms/cpp/tests/test_p0002.cpp)

## 实现复用与边界检查

哑节点可放在局部栈上。若用 new 分配临时哑节点，返回 next 前必须处理它的生命周期；结果链表也需要明确的回收者。此实现采用局部哑节点，测试显式销毁新结果。

## 9. 本地验证

180组1..70位随机数字链，使用逐位数组竖式参照；核对输入未改、输出无原节点地址，结果显式销毁。 测试采用固定种子，可复现每轮输入。断言针对返回值，也针对应保持的地址、连接或输入状态。内存由arena或对应destroy工具明确回收。本地测试通过仅说明这些可复现用例通过，不等于完整在线评测结论。

从仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp
c++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0002.cpp -o /tmp/hot100-cpp/test_p0002
/tmp/hot100-cpp/test_p0002
# 编译器支持时，再检查地址、未定义行为与泄漏
c++ -std=c++17 -Wall -Wextra -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0002.cpp -o /tmp/hot100-cpp/test_p0002_san
ASAN_OPTIONS=detect_leaks=1 /tmp/hot100-cpp/test_p0002_san
# 若执行环境在ptrace下明确拒绝LeakSanitizer，可仅运行地址/UB检查：
# ASAN_OPTIONS=detect_leaks=0 /tmp/hot100-cpp/test_p0002_san
```

## 10. 面试延伸

- 如果数字按高位在前，需要怎样调整？栈或先反转副本，但不能未经允许破坏输入。
- 如果进制为 B，哪些式子改变？结果位用 total%B、进位用 total/B，并重新检查 total 的上界。

## 11. 两道练习与提示

1. 编写 base=16 的通用版本。提示：每一位要满足 0≤digit<base，carry 的范围需重新证明。
2. 把99个9与1相加。提示：不要转换为内置整数，用预期位序列验证长进位链。
