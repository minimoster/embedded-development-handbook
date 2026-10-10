# 208. 实现 Trie (前缀树) · C++17

> 学习位置：图论 · 第 4 课。难度：MEDIUM。这是一份独立编写的学习课，不是在线提交通过声明。

[官方题目](https://leetcode.cn/problems/implement-trie-prefix-tree/) · 本课用简短契约与独立推演讲解，完整输入约束以官方题页为准。

## 1. 先说清输入、输出与修改边界

Trie支持insert(word)、search(word)、startsWith(prefix)。字符限小写英文字母；search要求完整词已插入，startsWith只要求路径存在。重复插入不重复创建路径。本实现额外支持空串：空前缀恒为true，空词只有插入后search才为true。

## 2. 从直觉到可用方案

用字符串集合查完整词平均较快，但前缀查询往往要扫描所有词。Trie把公共前缀共享为从根出发的路径，每条边对应一个字符。查找长度L的前缀只走L条边，与词库里有多少词无关。

路径存在不等于完整词存在，节点必须带terminal标志。本实现用vector存节点、26个子节点下标表示边，而不是手工new许多裸指针。下标在vector扩容后仍有效，同时所有内存由容器自动回收。

## 3. 为什么正确：抓住不变量

每个节点与唯一根路径字符串对应，子边i增加字符'a'+i。插入沿路径复用已有节点或创建缺失节点，只在最后节点设terminal=true，因此terminal恰表示已插入完整词。search必须同时满足路径存在与terminal；startsWith只检查路径存在。共享前缀不改变其他完整词的terminal，故插入长词不会错误声明所有短前缀为词。

## 4. 动手过程

1. 节点含26个子下标，初始化为不存在的哨兵值，terminal=false；根下标0。
2. insert先验证所有字符，再逐字符查边；缺边时追加节点并写回父下标。
3. 末节点terminal=true。
4. lookup从根逐边走，缺边返回哨兵。
5. search检查末标志，startsWith检查lookup是否成功。

## 5. 原创例子逐步追踪

插入“cat”与“car”，根的c边共享，之后a边也共享，a节点分出t与r。search("ca")为false，startsWith("ca")为true；再insert("ca")只把a节点terminal设true，不新增节点。search("cab")为false，因为b边不存在。重复insert("cat")结构和结果不变。

## 6. 时间、栈与输出空间分别算

单次insert、search、startsWith对长度L字符串均为O(L)时间；insert字符验证增加一次线性扫描但阶数不变。查询辅助空间O(1)，无递归栈。设已分配节点数P≤1+累计插入字符数，持久空间O(26P)=O(P)，返回bool空间O(1)。vector扩容有偶发O(P)移动，按插入总量摊还为每新增节点O(1)，不能把每次insert最坏情况都称为严格O(L)。

## 7. C++17实现与所有权

- vector.emplace_back可能使引用/指针失效；先保存子下标到值变量，扩容后重新用父下标访问，不持有int&跨扩容。
- 使用size_t下标与max哨兵，避免节点数量转成int的截断风险。
- 全串验证后才修改结构，非法字符不会导致半插入；invalid_argument是本实现的扩展约定。
- vector值节点允许安全默认复制：下标只引用各自容器内的节点；不同于保存跨对象裸指针。
- 26分支仅适合固定小字符集；Unicode需要编码分词与不同孩子表结构。
- 容器析构负责回收，用户不应手动delete内部节点。

下面是本仓库完整的头文件实现；support.hpp只提供标准库包含与题目节点声明，算法自身不依赖测试工具。在线评测通常已提供节点类型，提交时保留对应类并使用评测环境要求的作用域。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0208 {
class Trie {
    static constexpr std::size_t missing = std::numeric_limits<std::size_t>::max();
    struct Entry {
        std::array<std::size_t, 26> children;
        bool terminal = false;
        Entry() { children.fill(missing); }
    };
    std::vector<Entry> nodes_{1};
    static void validate(const std::string& text) {
        for (char ch : text)
            if (ch < 'a' || ch > 'z') throw std::invalid_argument("Trie expects lowercase ASCII");
    }
    std::size_t lookup(const std::string& text) const {
        validate(text);
        std::size_t current = 0;
        for (char ch : text) {
            current = nodes_[current].children[static_cast<std::size_t>(ch - 'a')];
            if (current == missing) break;
        }
        return current;
    }
public:
    Trie() = default;
    void insert(const std::string& word) {
        validate(word);
        std::size_t current = 0;
        for (char ch : word) {
            std::size_t edge = static_cast<std::size_t>(ch - 'a');
            std::size_t next = nodes_[current].children[edge];
            if (next == missing) {
                next = nodes_.size();
                nodes_.emplace_back(); // may relocate storage; keep indexes, not references
                nodes_[current].children[edge] = next;
            }
            current = next;
        }
        nodes_[current].terminal = true;
    }
    bool search(const std::string& word) const {
        std::size_t found = lookup(word);
        return found != missing && nodes_[found].terminal;
    }
    bool startsWith(const std::string& prefix) const { return lookup(prefix) != missing; }
};
} // namespace hot100::p0208
```

[查看可编译源文件](../../../algorithms/cpp/solutions/p0208_implement_trie_prefix_tree.hpp) · [查看完整测试](../../../algorithms/cpp/tests/test_p0208.cpp)

## 实现复用与边界检查

裸指针分配的子树需要明确析构与回收；vector 值节点简化所有权，但扩容会使引用失效。非法字符与空串行为属于明确的接口扩展。

## 9. 本地验证

1600轮固定种子插入/查询与set词库参照比较，验证全部词及其每个前缀；扩容、重复词、空词、非法字符和独立复制。 测试采用固定种子，可复现每轮输入。断言针对返回值，也针对应保持的地址、连接或输入状态。内存由arena或对应destroy工具明确回收。本地测试通过仅说明这些可复现用例通过，不等于完整在线评测结论。

从仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp
c++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0208.cpp -o /tmp/hot100-cpp/test_p0208
/tmp/hot100-cpp/test_p0208
# 编译器支持时，再检查地址、未定义行为与泄漏
c++ -std=c++17 -Wall -Wextra -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0208.cpp -o /tmp/hot100-cpp/test_p0208_san
ASAN_OPTIONS=detect_leaks=1 /tmp/hot100-cpp/test_p0208_san
# 若执行环境在ptrace下明确拒绝LeakSanitizer，可仅运行地址/UB检查：
# ASAN_OPTIONS=detect_leaks=0 /tmp/hot100-cpp/test_p0208_san
```

## 10. 面试延伸

- 如何支持删除词？仅清terminal不会释放无用分支；可记录经过计数、终止计数并回收零计数节点。
- 如何查某前缀下有多少词？节点保存子树词计数，重复插入语义需明确。
- 路径压缩Trie为何能省内存？把只有单孩子的链压成多字符边，但匹配与拆边更复杂。

## 11. 两道练习与提示

1. 增加countPrefix(prefix)，让重复插入只计一次。提示：先判断词是否存在，再沿路径增加子树计数。
2. 写vector会频繁扩容的压力测试。提示：插入数千个不同词，检验所有词和前缀；用ASan捕获悬空引用。
