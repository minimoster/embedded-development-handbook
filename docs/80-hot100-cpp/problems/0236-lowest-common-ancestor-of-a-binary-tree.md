# 236 二叉树的最近公共祖先

> 热题 100 · 二叉树 · C++17 · Medium
> 训练目标：从子树状态、遍历顺序和节点所有权出发，写出可重复调用、可本地验证的解法。

## 题目契约

输入普通二叉树的根，以及已存在于树中的两个节点地址 p、q，返回它们最深的共同祖先地址。节点自身也算祖先；用地址判断身份，不能按值猜节点。官方合法输入保证两个目标都存在，本课也允许 p==q 的自然扩展。

- 官方题目：[力扣 236 二叉树的最近公共祖先](https://leetcode.cn/problems/lowest-common-ancestor-of-a-binary-tree/)
- 完整实现：[下载或查看头文件](../../../algorithms/cpp/solutions/p0236_lowest_common_ancestor_of_a_binary_tree.hpp)
- 本地测试：[查看测试源码](../../../algorithms/cpp/tests/test_p0236.cpp)

## 从直觉到算法

先求根到 p、q 的两条路径，再取公共前缀最后一项是可行基线，需要保存路径。后序递归可把子树搜索结果压缩成一个指针：没有目标返回空，发现目标向上传递目标，左右都收到目标则当前节点就是汇合点。树无须满足 BST，不能依据数值大小选择分支。

## 核心不变量与正确性

在两个目标确实存在的契约下：若当前节点就是 p 或 q，它本身是另一目标在其子树中时的最近公共祖先，也可以作为一个已找到目标向上传递。否则递归两边；两边均非空，说明目标分属左右，当前节点是它们最低汇合处；只有一边非空，则共同祖先或目标都在该边，继续上传该结果。孩子先处理保证深处的祖先优先返回。

## 操作步骤

1. 空子树返回空；当前节点地址等于 p 或 q 时直接返回当前节点。
2. 递归查询左子树与右子树。
3. 如果左右结果均非空，返回当前节点。
4. 否则返回非空的一侧，两侧都空则返回空。

## 手工走一遍

根 8，左孩子 3，右孩子 10；3 的孩子为 1、6，6 的左孩子为 4。查询 1 与 4 时，1 从左侧上传，4 经 6 从右侧上传，在 3 汇合，返回 3。查询 3 与 4 时，在 3 直接命中目标，所以返回 3，符合“自身也是祖先”。

## C++17 完整实现

以下代码与仓库头文件内容一致。在线提交时平台通常已定义 `TreeNode`；本地通过 support.hpp 提供节点类型及标准库。命名空间用于同时编译多个题目的 Solution，不是算法要求。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0236 {
class Solution {
public:
    TreeNode* lowestCommonAncestor(TreeNode* root, TreeNode* p, TreeNode* q) {
        if (root == nullptr || root == p || root == q) return root;
        TreeNode* left = lowestCommonAncestor(root->left, p, q);
        TreeNode* right = lowestCommonAncestor(root->right, p, q);
        if (left != nullptr && right != nullptr) return root;
        return left != nullptr ? left : right;
    }
};
} // namespace hot100::p0236
```

## 复杂度拆解

最坏时间 O(n)，命中目标可跳过其后代。除栈外辅助空间 O(1)，递归栈 O(h)，输出为借用指针占 O(1)。不像两条完整路径基线，本实现不另存 O(h) 路径数组。

## C++ 工程与边界陷阱

- 返回的是树内原节点地址，不是新节点；不能对它单独 delete。
- 必须比较 `root==p` 而非 `root->val==p->val`，即使题目保证唯一值，按身份编写接口也更稳健。
- 如果某个目标不在树里，该算法可能返回唯一找到的目标；它不验证存在性，不能把该结果当作通用缺失目标处理。
- 空树与非空目标不满足合法契约，函数自然返回空，但不代表完成了目标存在验证。

## 实现复用与边界检查

目标直接返回、左右结果汇合的递归遵循经典 LCA 思路。必须明确节点身份、结果的借用所有权及缺失目标是否在契约内。

## 面试追问

- 若允许目标缺失，怎么避免误报？辅助结果同时返回已找到目标数和候选祖先，入口只有找到两个目标才返回候选；p==q 需单独定义计数。
- 若有父指针？像链表相交一样对齐深度，或者从一个节点向上保存祖先集合。
- 同一棵树大量查询怎样做？预处理二进制提升表，查询 O(log n)，代价是 O(n log n) 存储。

## 两道练习与具体提示

1. 实现允许目标缺失的版本。提示：每个子树返回“找到 p/q 的两个布尔标记+候选”，入口核验两个标记。
2. 给节点维护 parent，再用 O(1) 辅助空间寻找 LCA。提示：先算两者深度，较深者先爬到同层，然后同时向上。

## 本地验证

在仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp-build
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0236.cpp -o /tmp/hot100-cpp-build/p0236_lowest_common_ancestor_of_a_binary_tree
/tmp/hot100-cpp-build/p0236_lowest_common_ancestor_of_a_binary_tree
```

内存与未定义行为检查（编译器支持时）：

```bash
g++ -std=c++17 -Wall -Wextra -pedantic -g -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0236.cpp -o /tmp/hot100-cpp-build/p0236_lowest_common_ancestor_of_a_binary_tree_san
ASAN_OPTIONS=detect_leaks=1 /tmp/hot100-cpp-build/p0236_lowest_common_ancestor_of_a_binary_tree_san
```

测试用 assert 验证正常场景、边界与对象复用；源码中的固定随机种子使差分或性质检查可复现。输入节点由 TreeArena 管理；动态新建的输出树由 destroyTree 释放，二者的释放方式不可混用。测试过程不联网，也不作在线 AC 声明。
