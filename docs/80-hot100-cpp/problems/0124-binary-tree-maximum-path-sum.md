# 124 二叉树中的最大路径和

> 热题 100 · 二叉树 · C++17 · Hard
> 训练目标：从子树状态、遍历顺序和节点所有权出发，写出可重复调用、可本地验证的解法。

## 题目契约

在非空树中找一条至少包含一个节点的简单路径，使节点值之和最大。可从任意节点开始、结束，允许在某个节点连接左右两支，但不能重复节点。空树不在官方契约内，本地实现对空根抛 `invalid_argument`。

- 官方题目：[力扣 124 二叉树中的最大路径和](https://leetcode.cn/problems/binary-tree-maximum-path-sum/)
- 完整实现：[下载或查看头文件](../../../algorithms/cpp/solutions/p0124_binary_tree_maximum_path_sum.hpp)
- 本地测试：[查看测试源码](../../../algorithms/cpp/tests/test_p0124.cpp)

## 从直觉到算法

枚举所有起点与终点并求路径和，至少要做二次规模工作。后序遍历把信息分成两类：向父节点返回“从当前根出发，只选一侧继续的最大收益”；内部更新“以当前根为最高点，左右两侧都可使用的完整路径”。负收益分支可以不选，但节点自身必须选，才能处理全负树。

## 核心不变量与正确性

设左右返回的单向收益为 gL、gR，取 `L=max(0,gL)`、`R=max(0,gR)`。以当前节点封口的最佳路径是 `value+L+R`；所有路径都有唯一最高节点，因此全局取这些候选的最大值即为答案。返回父节点只能给 `value+max(L,R)`，因为父节点再连过来时，如果已经包含两侧，就会让当前节点有三条路径边，不再是一条简单路径。

## 操作步骤

1. 空孩子向上提供 0 贡献。
2. 后序计算左右单向收益，把负数截断为 0。
3. 用当前值加两侧非负贡献更新本次调用的全局最佳。
4. 仅返回当前值加较大一侧。入口用 long long 最小值初始化最佳，非空树必会更新它。

## 手工走一遍

根 -5，左孩子 2，右孩子 8；8 的孩子为 4、6。节点 8 的完整候选为 `8+4+6=18`，但只能向根返回 `8+6=14`。根处候选为 `-5+2+14=11`，不超过 18，所以答案是 `4→8→6`。另取单节点 -7，候选 -7 会更新初始负无穷，不能返回 0 空路径。

## C++17 完整实现

以下代码与仓库头文件内容一致。在线提交时平台通常已定义 `TreeNode`；本地通过 support.hpp 提供节点类型及标准库。命名空间用于同时编译多个题目的 Solution，不是算法要求。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0124 {
class Solution {
    static long long gain(TreeNode* node, long long& best) {
        if (node == nullptr) return 0;
        const long long left = std::max(0LL, gain(node->left, best));
        const long long right = std::max(0LL, gain(node->right, best));
        const long long value = node->val;
        best = std::max(best, value + left + right);
        return value + std::max(left, right);
    }
public:
    int maxPathSum(TreeNode* root) {
        if (root == nullptr) throw std::invalid_argument("maximum path requires a node");
        long long best = std::numeric_limits<long long>::lowest();
        gain(root, best);
        return static_cast<int>(best);
    }
};
} // namespace hot100::p0124
```

## 复杂度拆解

时间 O(n)；除调用栈外辅助空间 O(1)，递归栈 O(h)。输出为整数，占 O(1)。中间和使用 long long；[官方约束](https://leetcode.cn/problems/binary-tree-maximum-path-sum/)节点数至多 30000、绝对节点值至多 1000，故答案绝对值不超过 30000000，最终转回 int 安全。若扩展约束，需保留 long long 返回或检查转换范围。

## C++ 工程与边界陷阱

- 全负树的答案应是最大那个节点值，不能把全局 best 初始化为 0。
- 返回单向收益与更新双向候选是不同操作，混用会生成非法分叉路径。
- best 是入口局部变量，重复调用不会保留上次更大的结果。
- `node->val` 先转成 long long 再加，避免 int 加法发生溢出后才转换；不修改、不释放输入节点。

## 实现复用与边界检查

最大路径和必须保留全负树的非空路径语义。每次调用重置最优值，使用 max(0,gain) 表达可舍弃负分支，避免旧答案残留。

## 面试追问

- 怎样输出最优路径？保存更新 best 的拐点，并记录每个节点选择了哪条非负单向分支；最后拼出两端。
- 如果路径必须从根到叶子，能继续截掉负分支吗？不能，叶子约束会强迫选择一条完整下行路线，需要改变状态定义。
- 能否使用尾递归？必须等待两侧结果，一般不能直接尾递归化；显式后序帧可规避调用栈。

## 两道练习与具体提示

1. 返回最大路径的节点列表。提示：单向状态附带端点或选择方向；best 更新时记录拐点与两边选择。
2. 求最小路径和，路径仍至少一个节点。提示：将有利贡献改为 `min(0,gain)`，初始化全局值为正无穷，返回较小单侧。

## 本地验证

在仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp-build
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0124.cpp -o /tmp/hot100-cpp-build/p0124_binary_tree_maximum_path_sum
/tmp/hot100-cpp-build/p0124_binary_tree_maximum_path_sum
```

内存与未定义行为检查（编译器支持时）：

```bash
g++ -std=c++17 -Wall -Wextra -pedantic -g -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0124.cpp -o /tmp/hot100-cpp-build/p0124_binary_tree_maximum_path_sum_san
ASAN_OPTIONS=detect_leaks=1 /tmp/hot100-cpp-build/p0124_binary_tree_maximum_path_sum_san
```

测试用 assert 验证正常场景、边界与对象复用；源码中的固定随机种子使差分或性质检查可复现。输入节点由 TreeArena 管理；动态新建的输出树由 destroyTree 释放，二者的释放方式不可混用。测试过程不联网，也不作在线 AC 声明。
