# 94 二叉树的中序遍历

> 热题 100 · 二叉树 · C++17 · Easy
> 训练目标：从子树状态、遍历顺序和节点所有权出发，写出可重复调用、可本地验证的解法。

## 题目契约

输入根指针 `root`，输出按“左子树 → 当前节点 → 右子树”排列的值数组。空树返回空数组；节点值可重复。函数只读取指针与值，不修改树。中序遍历并不自动得到排序结果，只有二叉搜索树才有这个性质。

- 官方题目：[力扣 94 二叉树的中序遍历](https://leetcode.cn/problems/binary-tree-inorder-traversal/)
- 完整实现：[下载或查看头文件](../../../algorithms/cpp/solutions/p0094_binary_tree_inorder_traversal.hpp)
- 本地测试：[查看测试源码](../../../algorithms/cpp/tests/test_p0094.cpp)

## 从直觉到算法

最直接的递归把三件事写在函数里，但深链会消耗系统调用栈。若先把整棵树复制到数组，再排序，会同时破坏一般二叉树的访问顺序与增加开销。本题真正需要的是记住“左边还没完成，因此父节点暂不能输出”的那些节点。用显式栈模拟递归帧，仍然只访问每个节点一次，而且可以清楚看到访问时机。

## 核心不变量与正确性

内层循环把当前子树的左脊柱压栈；栈顶是尚未输出节点中，中序位置最靠前的一个。弹出时，它的左子树已全部处理，所以此刻输出正确。转向右子树之后，用同一规则处理右子树，恰好恢复递归定义。每个非空节点只入栈、出栈各一次，因此既不漏节点，也不重复。

## 操作步骤

1. 准备结果数组与保存 `TreeNode*` 的栈，令游标指向根。
2. 游标非空时，先压入当前节点，然后一直走左孩子。
3. 左边走到空后，弹出栈顶，记录它的值。
4. 将游标改为刚弹出节点的右孩子。只有游标为空且栈也空，遍历才结束。

## 手工走一遍

取根 6，左孩子 2，2 的右孩子 4，右孩子 9。先压入 6、2；2 没有左孩子，于是输出 2。随后压入 4 并输出 4，栈里仍有 6。输出 6 后转到 9，最终数组为 `[2,4,6,9]`。栈保存的是返回位置，值为 2 的节点已经输出后不会再次入栈。

## C++17 完整实现

以下代码与仓库头文件内容一致。在线提交时平台通常已定义 `TreeNode`；本地通过 support.hpp 提供节点类型及标准库。命名空间用于同时编译多个题目的 Solution，不是算法要求。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0094 {
class Solution {
public:
    std::vector<int> inorderTraversal(TreeNode* root) {
        std::vector<int> answer;
        std::vector<TreeNode*> pending;
        TreeNode* cursor = root;
        while (cursor != nullptr || !pending.empty()) {
            while (cursor != nullptr) {
                pending.push_back(cursor);
                cursor = cursor->left;
            }
            cursor = pending.back();
            pending.pop_back();
            answer.push_back(cursor->val);
            cursor = cursor->right;
        }
        return answer;
    }
};
} // namespace hot100::p0094
```

## 复杂度拆解

设节点数为 n、树高为 h。时间 O(n)；显式栈辅助空间 O(h)，最坏退化树为 O(n)。递归栈空间 O(1)，因为实现没有递归；返回数组占 O(n) 输出空间。

## C++ 工程与边界陷阱

- 栈保存借用指针，不取得节点所有权，不应对栈中的指针 `delete`。
- 必须先弹出再访问右孩子；一直读取栈顶却不弹出会陷入循环。
- 不用数值哨兵，所以 `INT_MIN`、重复值均可正常输出。
- 本实现避免调用栈溢出，但 n 个结果仍必须实际占用内存。

## 实现复用与边界检查

递归中序和显式栈都遵循左、根、右顺序。显式栈帮助练习递归状态转换与深树栈管理，空间分析仍需计入树高。

## 面试追问

- 如果输出改成迭代器，如何把“下一次访问位置”保留在对象中？保留栈和游标即可，每次调用只推进到一次弹栈。
- 能否做到 O(1) 辅助空间？Morris 遍历可以临时建立线索，但必须恢复指针，且不适合同时有只读访问者的树。
- 为什么不能用层序遍历替代？层序按深度排序，中序按子树结构排序，二者的顺序约束不同。

## 两道练习与具体提示

1. 改为前序遍历，仍使用显式栈。提示：弹出时立即输出，先压右孩子，再压左孩子。
2. 做一个中序迭代器，提供 `hasNext()` 与 `next()`。提示：抽出“压入整条左脊柱”的辅助函数；`next()` 弹出后压入其右子树左脊柱。

## 本地验证

在仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp-build
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0094.cpp -o /tmp/hot100-cpp-build/p0094_binary_tree_inorder_traversal
/tmp/hot100-cpp-build/p0094_binary_tree_inorder_traversal
```

内存与未定义行为检查（编译器支持时）：

```bash
g++ -std=c++17 -Wall -Wextra -pedantic -g -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0094.cpp -o /tmp/hot100-cpp-build/p0094_binary_tree_inorder_traversal_san
ASAN_OPTIONS=detect_leaks=1 /tmp/hot100-cpp-build/p0094_binary_tree_inorder_traversal_san
```

测试用 assert 验证正常场景、边界与对象复用；源码中的固定随机种子使差分或性质检查可复现。输入节点由 TreeArena 管理；动态新建的输出树由 destroyTree 释放，二者的释放方式不可混用。测试过程不联网，也不作在线 AC 声明。
