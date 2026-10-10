# 437 路径总和 III

> 热题 100 · 二叉树 · C++17 · Medium
> 训练目标：从子树状态、遍历顺序和节点所有权出发，写出可重复调用、可本地验证的解法。

## 题目契约

统计和为 targetSum 的向下连续路径数量。路径可从任意节点开始、在任意节点结束，但不能从子节点向上再转到另一分支。值可为负或零，因此不能用“超过目标就剪枝”。输入只读。

- 官方题目：[力扣 437 路径总和 III](https://leetcode.cn/problems/path-sum-iii/)
- 完整实现：[下载或查看头文件](../../../algorithms/cpp/solutions/p0437_path_sum_iii.hpp)
- 本地测试：[查看测试源码](../../../algorithms/cpp/tests/test_p0437.cpp)

## 从直觉到算法

基线从每个节点出发递归累加，最坏链状树 O(n²)。把根到当前节点的前缀和记为 S；若某个祖先之前的前缀为 S-target，这段祖先之后到当前节点的路径和就等于 target。DFS 中只维护当前根路径的前缀计数，就能一次查询统计所有以当前节点结尾的合法路径。

## 核心不变量与正确性

进入节点时，频次表只包含当前节点之前这条祖先路径的前缀和，另有虚拟空前缀 0 的一次计数。先查询 `S-target`，每一个匹配前缀代表一个不同起点；然后把 S 加入表，供孩子查询。离开节点前把 S 撤销，避免兄弟分支互相配对。查询在插入之前，保证 target=0 时不会把空路径算进结果。

## 操作步骤

1. 建立 `frequency[0]=1`，表示从根开始的路径也能匹配。
2. 进入节点后把其值加入 long long 前缀和。
3. 查询 `prefix-target` 的次数，累加答案；再增加当前 prefix 次数。
4. 遍历左右孩子，最后撤销本节点前缀；计数降到零时擦除键，控制表内只保留当前路径。

## 手工走一遍

根 3，左孩子 -1，-1 的孩子为 2、1，右孩子 2；目标为 2。访问 -1 时前缀为 2，匹配虚拟 0，得到路径 `[3,-1]`。访问左侧 2 时前缀为 4，匹配父位置前缀 2，得到 `[2]`。回溯撤销左分支后访问右侧 2，前缀为 5，匹配根前缀 3，得到另一条 `[2]`。总共 3 条，绝不跨接左右兄弟。

## C++17 完整实现

以下代码与仓库头文件内容一致。在线提交时平台通常已定义 `TreeNode`；本地通过 support.hpp 提供节点类型及标准库。命名空间用于同时编译多个题目的 Solution，不是算法要求。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0437 {
class Solution {
    static long long count(TreeNode* node, long long prefix, long long target,
                           std::unordered_map<long long, int>& frequency) {
        if (node == nullptr) return 0;
        prefix += node->val;
        long long answer = 0;
        const auto found = frequency.find(prefix - target);
        if (found != frequency.end()) answer += found->second;
        ++frequency[prefix];
        answer += count(node->left, prefix, target, frequency);
        answer += count(node->right, prefix, target, frequency);
        auto current = frequency.find(prefix);
        if (--current->second == 0) frequency.erase(current);
        return answer;
    }
public:
    int pathSum(TreeNode* root, int targetSum) {
        std::unordered_map<long long, int> frequency{{0LL, 1}};
        return static_cast<int>(count(root, 0LL, targetSum, frequency));
    }
};
} // namespace hot100::p0437
```

## 复杂度拆解

哈希期望时间 O(n)，碰撞退化时最坏 O(n²)。因为撤销后擦除零计数，表内最多 O(h) 个活动前缀；递归栈 O(h)，其他辅助空间 O(1)，输出整数 O(1)。若只减计数而不擦除，键可能积累到 O(n)，不能继续声称哈希表空间 O(h)。

## C++ 工程与边界陷阱

- 前缀与目标差使用 long long，多个 int 值的路径和可能超过 int；`long` 不跨平台保证 64 位。
- 计数表必须有“加入、递归、撤销”的对称动作，漏撤销会把不同分支错误地连成路径。
- [官方约束](https://leetcode.cn/problems/path-sum-iii/) n<=1000 时路径总数最多 n(n+1)/2=500500，int 返回值安全；内部总数仍用 long long。更大通用接口应改返回类型。
- 不保存 map 迭代器跨递归：孩子插入可能触发 rehash。撤销时重新 find 当前前缀。

## 实现复用与边界检查

前缀和中间类型要覆盖合法累加范围；long 在部分平台仅32位。回溯后删除零计数键可限制保留状态，但零键本身不导致计数错误。额外极端回归输入与官方范围应分开记录。

## 面试追问

- 若要求输出所有具体路径，复杂度会怎样？即便计数是线性，输出规模也可能二次甚至路径元素总量三次，须单独计输出成本。
- 为什么负数阻止滑动窗口？增加一项可能减小和，移除一项也可能增大和，单调性消失。
- 是否能并行处理两边？可以，但各分支必须拥有独立前缀状态或持久化结构，不能直接共享可变 map。

## 两道练习与具体提示

1. 输出目标路径的节点值列表。提示：记录当前路径数组与每个前缀对应的位置集合，匹配时复制相应后缀。
2. 将哈希表换成 `std::map` 并分析代价。提示：活动键不超过 h，查找与更新 O(log(h+1))，总时间 O(n log(h+1))。

## 本地验证

在仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp-build
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0437.cpp -o /tmp/hot100-cpp-build/p0437_path_sum_iii
/tmp/hot100-cpp-build/p0437_path_sum_iii
```

内存与未定义行为检查（编译器支持时）：

```bash
g++ -std=c++17 -Wall -Wextra -pedantic -g -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0437.cpp -o /tmp/hot100-cpp-build/p0437_path_sum_iii_san
ASAN_OPTIONS=detect_leaks=1 /tmp/hot100-cpp-build/p0437_path_sum_iii_san
```

测试用 assert 验证正常场景、边界与对象复用；源码中的固定随机种子使差分或性质检查可复现。输入节点由 TreeArena 管理；动态新建的输出树由 destroyTree 释放，二者的释放方式不可混用。测试过程不联网，也不作在线 AC 声明。
