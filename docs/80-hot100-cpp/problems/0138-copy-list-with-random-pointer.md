# 138. 随机链表的复制 · C++17

> 学习位置：链表 · 第 11 课。难度：MEDIUM。这是一份独立编写的学习课，不是在线提交通过声明。

[官方题目](https://leetcode.cn/problems/copy-list-with-random-pointer/) · 本课用简短契约与独立推演讲解，完整输入约束以官方题页为准。

## 1. 先说清输入、输出与修改边界

复制一条 next 无环的链表；每个 random 指向原链中的节点或 nullptr。返回全新节点组成的等价链，复制链的 next/random 都不能指向原节点，原链保持不变。

## 2. 从直觉到可用方案

只按 next 新建节点很容易，但 random 可以指向尚未创建的节点，也可能自指或形成随机边环。复制值并不能表达这些连接。

先建立“原地址→新地址”的一一映射，再复制边，是图复制的通用模式。第一遍只造节点，第二遍才连 next 与 random，这样任意随机目标都已存在。交织新旧节点可节省哈希表，但会暂时修改输入；这里选择不改输入且更易审查的两遍法。

## 3. 为什么正确：抓住不变量

第一遍结束后，每个原节点恰好对应一个独立的新节点，值相同且尚未连边。第二遍对原节点u的每条边u→v设置clone(u)→clone(v)，为空则复制为空，因此原图的地址关系被映射完整保持。新节点全部由独立分配得到，任何复制边都不会泄露到原链。

## 4. 动手过程

1. 空头直接返回 nullptr。
2. 用 unordered_map<Node*,unique_ptr<Node>> 创建所有新节点，临时由unique_ptr拥有。
3. 再次遍历原链，通过映射设置每个副本的next和random。
4. 保存复制头，全部release后把所有权交给调用者。
5. 调用者沿复制next使用destroyRandomList回收，不沿random递归释放。

## 5. 原创例子逐步追踪

原链为 A(4)→B(4)→C(9)，random分别是C、A、C。第一遍得到A′、B′、C′三个不同地址。第二遍连接A′.next=B′、A′.random=C′，B′.random=A′，C′.random=C′。两个值为4的节点仍对应两个独立副本，C的自指也必须成为C′的自指。

## 6. 时间、栈与输出空间分别算

哈希查找平均O(1)，平均总时间O(n)；极端碰撞下总时间可退化为O(n²)。映射辅助空间O(n)，无递归栈；新增输出节点空间O(n)。暂存unique_ptr与最后release不复制节点，不改变复杂度。

## 7. C++17实现与所有权

- 映射键是Node*，不是val；重复值非常常见。
- 对nullptr显式分支，避免operator[]悄悄插入一个无意义的空键。
- unique_ptr在构造或接线异常时自动清理临时副本；全部连好后统一交出所有权。
- random边可能成环，但next链无环。销毁副本只沿next；沿random递归会重复释放。
- random指向链外属于契约外输入，at会暴露异常，不能默默生成不存在的副本。

下面是本仓库完整的头文件实现；support.hpp只提供标准库包含与题目节点声明，算法自身不依赖测试工具。在线评测通常已提供节点类型，提交时保留对应类并使用评测环境要求的作用域。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0138 {
class Solution {
public:
    Node* copyRandomList(Node* head) {
        if (!head) return nullptr;
        std::unordered_map<Node*, std::unique_ptr<Node>> copies;
        for (Node* p = head; p; p = p->next)
            copies.emplace(p, std::make_unique<Node>(p->val));
        for (Node* p = head; p; p = p->next) {
            Node* copy = copies.at(p).get();
            copy->next = p->next ? copies.at(p->next).get() : nullptr;
            copy->random = p->random ? copies.at(p->random).get() : nullptr;
        }
        Node* answer = copies.at(head).get();
        for (auto& entry : copies) entry.second.release();
        return answer;
    }
};
} // namespace hot100::p0138
```

[查看可编译源文件](../../../algorithms/cpp/solutions/p0138_copy_list_with_random_pointer.hpp) · [查看完整测试](../../../algorithms/cpp/tests/test_p0138.cpp)

## 实现复用与边界检查

两遍映射是有效方法。operator[] 用于查找空指针会插入空映射，查找与插入宜明确区分；此实现显式处理空边，用 unique_ptr 暂存所有权。

## 9. 本地验证

160组随机next链与随机目标（空、前指、后指、自指），核对复制值、随机目标下标、地址不共享和输入未修改；额外自环random单节点。 测试采用固定种子，可复现每轮输入。断言针对返回值，也针对应保持的地址、连接或输入状态。内存由arena或对应destroy工具明确回收。本地测试通过仅说明这些可复现用例通过，不等于完整在线评测结论。

从仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp
c++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0138.cpp -o /tmp/hot100-cpp/test_p0138
/tmp/hot100-cpp/test_p0138
# 编译器支持时，再检查地址、未定义行为与泄漏
c++ -std=c++17 -Wall -Wextra -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0138.cpp -o /tmp/hot100-cpp/test_p0138_san
ASAN_OPTIONS=detect_leaks=1 /tmp/hot100-cpp/test_p0138_san
# 若执行环境在ptrace下明确拒绝LeakSanitizer，可仅运行地址/UB检查：
# ASAN_OPTIONS=detect_leaks=0 /tmp/hot100-cpp/test_p0138_san
```

## 10. 面试延伸

- 交织法为什么是O(1)辅助空间但仍有O(n)输出空间？新节点是必须的结果，不能计成“没有用空间”。
- 若next也能成环，应怎样复制？需要访问集合与图遍历，不能再用while(node)作为结束条件。

## 11. 两道练习与提示

1. 实现交织法，并检查原链next完全恢复。提示：新节点放在原节点后面，random目标副本就在旧目标的next。
2. 给复制结果建立random目标下标序列，与输入比较。提示：先分别把两条next链编号，再把random地址转成编号。
