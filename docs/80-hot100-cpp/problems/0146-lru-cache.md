# 146. LRU 缓存 · C++17

> 学习位置：链表 · 第 14 课。难度：MEDIUM。这是一份独立编写的学习课，不是在线提交通过声明。

[官方题目](https://leetcode.cn/problems/lru-cache/) · 本课用简短契约与独立推演讲解，完整输入约束以官方题页为准。

## 1. 先说清输入、输出与修改边界

实现固定容量LRUCache：get命中返回值并把键设为最近使用，未命中返回-1；put更新或插入键，容量超出时移除最久未用键。官方容量为正；本实现支持容量0，此时不保留任何键。

## 2. 从直觉到可用方案

只用哈希表能快速查键，却不能快速找最久未用项；只用顺序链表能维护访问顺序，但查键要O(c)。把两者组合：双向list按最近到最久排序，unordered_map把键映射到list迭代器。

命中后用splice将已有节点移到表头，不复制节点，也不使对应迭代器失效。尾部就是淘汰候选。使用标准容器，避免手写prev/next、哑头尾和析构。

## 3. 为什么正确：抓住不变量

始终保持三个条件：entries按最近访问降序排列；每个entries节点在index中有唯一匹配项；entries.size()≤capacity。get命中或更新put把节点移到头并保持映射；新put先在头插入并记录映射，超容量时删除尾键的映射与节点。三种操作均保持这三个不变量，因此尾节点确为当前LRU。

## 4. 动手过程

1. entries存pair<key,value>，index存key到list iterator。
2. get查find；未命中不改变顺序，命中splice到begin并返回value。
3. put已有键：更新value，再splice到begin。
4. put新键：头插节点并建索引；若索引分配失败，回滚刚插的list节点。
5. 超容量时先erase尾键索引，再pop_back；容量0直接返回。

## 5. 原创例子逐步追踪

容量2：put(4,40)后顺序[4]；put(8,80)后[8,4]；get(4)=40，顺序[4,8]；put(5,50)变成[5,4,8]，淘汰8，剩[5,4]；put(4,41)后[4,5]；get(8)=-1，不改变顺序。更新已有键不会增加容量占用。

## 6. 时间、栈与输出空间分别算

容量为c：list的splice、头插、尾删均O(1)，unordered_map查找与删除平均O(1)，故get/put平均O(1)。极端哈希碰撞下单次可能O(c)。存储空间O(c)，返回值O(1)，没有递归栈。rehash可能造成单次非恒定开销，平均复杂度不等于严格最坏常数。

## 7. C++17实现与所有权

- std::list迭代器在同一list内splice后仍有效，unordered_map中的位置无需重写。
- 先删除尾键索引再删除list节点，避免保留悬空迭代器。
- 类禁止复制与移动：默认复制会把index里的迭代器复制成仍指向旧list的地址，破坏独立性；需要复制时应重建索引。
- 标准容器负责释放节点，无需析构中手动delete。
- insert失败时回滚list节点，使逻辑状态保持一致；内存分配失败不属于官方测试重点，但这是容器组合的重要工程细节。

下面是本仓库完整的头文件实现；support.hpp只提供标准库包含与题目节点声明，算法自身不依赖测试工具。在线评测通常已提供节点类型，提交时保留对应类并使用评测环境要求的作用域。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0146 {
class LRUCache {
    using Entries = std::list<std::pair<int, int>>;
    std::size_t capacity_;
    Entries entries_;
    std::unordered_map<int, Entries::iterator> index_;
public:
    explicit LRUCache(int capacity)
        : capacity_(capacity > 0 ? static_cast<std::size_t>(capacity) : 0) {}
    LRUCache(const LRUCache&) = delete;
    LRUCache& operator=(const LRUCache&) = delete;
    LRUCache(LRUCache&&) = delete;
    LRUCache& operator=(LRUCache&&) = delete;
    int get(int key) {
        auto found = index_.find(key);
        if (found == index_.end()) return -1;
        entries_.splice(entries_.begin(), entries_, found->second);
        return found->second->second;
    }
    void put(int key, int value) {
        if (capacity_ == 0) return;
        auto found = index_.find(key);
        if (found != index_.end()) {
            found->second->second = value;
            entries_.splice(entries_.begin(), entries_, found->second);
            return;
        }
        entries_.emplace_front(key, value);
        try { index_.emplace(key, entries_.begin()); }
        catch (...) { entries_.pop_front(); throw; }
        if (entries_.size() > capacity_) {
            index_.erase(entries_.back().first);
            entries_.pop_back();
        }
    }
};
} // namespace hot100::p0146
```

[查看可编译源文件](../../../algorithms/cpp/solutions/p0146_lru_cache.hpp) · [查看完整测试](../../../algorithms/cpp/tests/test_p0146.cpp)

## 实现复用与边界检查

手写链表的缓存对象销毁时也要释放存活节点与哑节点。list+unordered_map 方案使用 RAII，但默认复制含迭代器索引的对象仍有风险。

## 9. 本地验证

容量0..8各3500个随机操作，和O(c)vector缓存逐次比较；额外核对命中提权、更新和淘汰顺序。 测试采用固定种子，可复现每轮输入。断言针对返回值，也针对应保持的地址、连接或输入状态。内存由arena或对应destroy工具明确回收。本地测试通过仅说明这些可复现用例通过，不等于完整在线评测结论。

从仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp
c++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0146.cpp -o /tmp/hot100-cpp/test_p0146
/tmp/hot100-cpp/test_p0146
# 编译器支持时，再检查地址、未定义行为与泄漏
c++ -std=c++17 -Wall -Wextra -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0146.cpp -o /tmp/hot100-cpp/test_p0146_san
ASAN_OPTIONS=detect_leaks=1 /tmp/hot100-cpp/test_p0146_san
# 若执行环境在ptrace下明确拒绝LeakSanitizer，可仅运行地址/UB检查：
# ASAN_OPTIONS=detect_leaks=0 /tmp/hot100-cpp/test_p0146_san
```

## 10. 面试延伸

- 如何实现线程安全？对list与index的整个联合操作加锁，不能分别锁而暴露半完成状态。
- LRU与LFU有什么区别？频率策略需要额外维护频次桶，访问顺序仍常用于同频次淘汰。
- 若get返回引用而非int，淘汰后引用生命周期应怎样约定？

## 11. 两道练习与提示

1. 编写O(c)的vector参照缓存，与随机get/put序列逐步对比。提示：命中与更新都移到数组头，未命中不改变顺序。
2. 实现正确的复制构造。提示：先复制list，再遍历新list重建index，不能直接复制旧迭代器。
