# 155. 最小栈：C++17 推导与实现

> 分类：栈 · 难度：MEDIUM · 官方 Hot 100 顺序：70/100

## 题意与调用约定

实现 push、pop、top、getMin；查询当前所有栈元素的最小值不需要扫描。官方保证 pop/top/getMin 的调用有效；这里额外给空栈调用明确异常。

题面与最新约束见 [力扣中文官方题目](https://leetcode.cn/problems/min-stack/) / [LeetCode 官方英文题目](https://leetcode.com/problems/min-stack/)。下文给出独立编写的推导、示例与实现；测试是本地验证，没有声称在线评测已通过。

## 从直觉到优化

每次 getMin 扫描会使查询 O(n)。关键是 pop 必须恢复“压入当前元素之前”的最小值，所以不能只存一个全局 minimum。本实现给每个栈位置附带该前缀最小值：空间换掉重新计算。重复最小值自然得到独立快照。

## 不变量与正确性

items[k].first 是第k个栈值，items[k].second 是第0到k个值的最小值。push 使用 min(新值,旧前缀最小值) 扩展该性质；pop 直接删除最后一对，前一个快照自动恢复。故栈顶的 second 始终是当前最小值。

## 算法步骤

1. 每次 push 计算新前缀最小值，并把值和快照同时压入。
2. pop 同时移除值与该层快照。
3. top 读取 first，getMin 读取 second。
4. 对无数据的查询和弹出先执行保护检查。

## 手工追踪

依次 push 4,-2,-2,7，存储 [(4,4),(-2,-2),(-2,-2),(7,-2)]。弹出7和一个-2后最小值仍是-2；再次弹出-2后，剩(4,4)，getMin恢复4。无需特判重复最小值。

## 完整 C++17 实现

以下内容来自本课 [可编译头文件](../../../algorithms/cpp/solutions/p0155_min_stack.hpp)，namespace用于让100题可同时进入同一工程。在线提交时一般只保留评测要求的类声明，并按平台要求处理标准库命名；本地辅助头文件负责统一导入标准库。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0155 {
class MinStack {
    std::deque<std::pair<int,int>> items;
    void requireData() const {
        if (items.empty()) throw std::out_of_range("empty MinStack");
    }
public:
    void push(int val) {
        int minimum = items.empty() ? val : std::min(val, items.back().second);
        items.emplace_back(val, minimum);
    }
    void pop() { requireData(); items.pop_back(); }
    int top() { requireData(); return items.back().first; }
    int getMin() { requireData(); return items.back().second; }
};
} // namespace hot100::p0155
```

## 复杂度核算

push/pop/top/getMin 在标准容器复杂度模型下均为 O(1)；deque 两端插入不需要整体迁移元素。总空间 O(n)，每项两个 int。若改用 vector，push 只有摊还 O(1)，扩容单次为 O(n)，应准确说明区别。

## C++ 常见陷阱

- 差值压栈方案 val-min 在 INT_MIN 与 INT_MAX 间可能溢出；快照法没有算术差值。
- deque 两端插入可能使迭代器失效；本实现不跨 push 保存迭代器，也不在 pop 后引用已删元素。
- 不要用初始最小值0，正数首元素会被错误比较；空栈首项直接使用 val。

## 测试与运行

[完整测试源文件](../../../algorithms/cpp/tests/test_p0155.cpp)：20000次固定种子随机压入/弹出操作，每次与普通vector的栈顶和扫描最小值核对；专测重复最小、极值与空查询异常。

从仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0155.cpp -o /tmp/hot100-cpp/test_p0155
/tmp/hot100-cpp/test_p0155
# 环境支持时增加运行期检查：
g++ -std=c++17 -Wall -Wextra -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0155.cpp -o /tmp/hot100-cpp/test_p0155_san
ASAN_OPTIONS=detect_leaks=0 /tmp/hot100-cpp/test_p0155_san
```

本次环境的LeakSanitizer受ptrace限制，因此命令显式关闭泄漏检测；AddressSanitizer与UndefinedBehaviorSanitizer仍启用。支持泄漏检测的本地环境可移除该环境变量。断言通过说明覆盖输入上的本地结果正确，不能替代证明、完整平台约束核对或在线评测。

## 面试追问

- 如何同时提供 getMax？每层再保存一个前缀最大值。
- 只保存“最小值变化”的辅助栈能节约哪些输入的空间？重复最小值怎样恢复？

## 练习与提示

- 练习1：实现两个栈版并对重复最小值测试。提示：相等时也要压入辅助栈，或保存出现次数。
- 练习2：做支持撤销最后一次 push/pop 的版本。提示：区分状态快照与操作日志。

## 实现复用与边界检查

C++17 不保证 long 为64位，LLP64 平台的 val-min 可溢出。差值栈仍需每项记录；此实现用前缀最小值快照，研究差值方案时应明确64位中间类型。
