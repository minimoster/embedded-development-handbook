# 543 二叉树的直径

> 热题 100 · 二叉树 · C++17 · Easy
> 训练目标：从子树状态、遍历顺序和节点所有权出发，写出可重复调用、可本地验证的解法。

## 题目契约

返回树中任意两节点之间最长简单路径的边数。路径可以完全位于某个子树，不必经过根。单节点与空树在本实现中都返回 0。

- 官方题目：[力扣 543 二叉树的直径](https://leetcode.cn/problems/diameter-of-binary-tree/)
- 完整实现：[下载或查看头文件](../../../algorithms/cpp/solutions/p0543_diameter_of_binary_tree.hpp)
- 本地测试：[查看测试源码](../../../algorithms/cpp/tests/test_p0543.cpp)

## 从直觉到算法

若对每个节点重新求左右子树高度，链状树上会形成 O(n²) 的重复工作。后序遍历允许一次获取两边高度并更新答案。需区分两个量：返回给父节点的单向高度，以及以当前节点为拐点的双向路径长度。

## 核心不变量与正确性

任意一条树路径都有唯一的最高节点 u。若左右子树节点高度为 L、R，经过 u 且分别深入两侧的路径有 L+R 条边：每侧从 u 出发的边数恰等于该侧节点高度。遍历每个 u 并取最大值覆盖所有路径。给父节点返回 `1+max(L,R)`，它只需要一条向下链，不能把双向直径当成高度。

## 操作步骤

1. 空子树高度为 0。
2. 递归求左右高度，确保先处理孩子再处理父节点。
3. 用 `left + right` 更新本次调用的最佳边数。
4. 返回 `1 + max(left,right)`；入口把最佳值初始化为 0。

## 手工走一遍

根 1 只有左孩子 2；2 的左链是 3→4，右链是 5→6。节点 2 得到左高 2、右高 2，候选直径为 4，对应 `4→3→2→5→6`。根 1 的候选仅为 3，所以答案 4，展示了最长路径并不经过根。

## C++17 完整实现

以下代码与仓库头文件内容一致。在线提交时平台通常已定义 `TreeNode`；本地通过 support.hpp 提供节点类型及标准库。命名空间用于同时编译多个题目的 Solution，不是算法要求。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0543 {
class Solution {
    static int height(TreeNode* node, int& best) {
        if (node == nullptr) return 0;
        const int left = height(node->left, best);
        const int right = height(node->right, best);
        best = std::max(best, left + right);
        return 1 + std::max(left, right);
    }
public:
    int diameterOfBinaryTree(TreeNode* root) {
        int best = 0;
        height(root, best);
        return best;
    }
};
} // namespace hot100::p0543
```

## 复杂度拆解

时间 O(n)；除栈外辅助空间 O(1)，递归栈 O(h)，输出空间 O(1)。此处高度以节点数计、直径以边数计，因此无需最后减一，也不会给空树返回负数。

## C++ 工程与边界陷阱

- `best` 是入口局部变量，通过引用传给辅助函数；每次调用都有独立答案。
- 本实现不改孩子指针，不拥有或释放输入节点。
- 对超出题目范围的巨大树，高度与边数可能超出 int；正常题目规模不会。
- 不要写成 `max(left,right)` 更新直径，这样会遗漏横跨两侧的路径。

## 实现复用与边界检查

路径节点数减一与直接维护边数均需明确空树语义。INT_MIN 再减一会发生有符号溢出；此实现直接维护边数并初始化为0，每次调用的最优值独立。

## 面试追问

- 若每条边有非负权重，如何改？返回最大向下边权和，候选是两侧贡献之和。
- 如何返回具体路径？在更新最佳值时保存拐点与两侧最深端点，再用父指针还原。
- 本题与最大路径和的共同点是什么？都区分“向上可接的一条链”和“在当前节点封口的完整路径”。

## 两道练习与具体提示

1. 同时返回直径长度及两个端点。提示：高度函数携带最深端点，更新直径时记录左右端点。
2. 改为计算最长路径包含的节点数。提示：非空树的候选是 `left+right+1`，空树需单独定义答案。

## 本地验证

在仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp-build
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0543.cpp -o /tmp/hot100-cpp-build/p0543_diameter_of_binary_tree
/tmp/hot100-cpp-build/p0543_diameter_of_binary_tree
```

内存与未定义行为检查（编译器支持时）：

```bash
g++ -std=c++17 -Wall -Wextra -pedantic -g -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0543.cpp -o /tmp/hot100-cpp-build/p0543_diameter_of_binary_tree_san
ASAN_OPTIONS=detect_leaks=1 /tmp/hot100-cpp-build/p0543_diameter_of_binary_tree_san
```

测试用 assert 验证正常场景、边界与对象复用；源码中的固定随机种子使差分或性质检查可复现。输入节点由 TreeArena 管理；动态新建的输出树由 destroyTree 释放，二者的释放方式不可混用。测试过程不联网，也不作在线 AC 声明。
