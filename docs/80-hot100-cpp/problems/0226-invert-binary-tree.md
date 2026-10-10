# 226 翻转二叉树

> 热题 100 · 二叉树 · C++17 · Easy
> 训练目标：从子树状态、遍历顺序和节点所有权出发，写出可重复调用、可本地验证的解法。

## 题目契约

原地交换每个节点的左右孩子，返回原根指针。空树返回空指针。节点值、节点地址与节点总数保持不变；变化的是连接关系。

- 官方题目：[力扣 226 翻转二叉树](https://leetcode.cn/problems/invert-binary-tree/)
- 完整实现：[下载或查看头文件](../../../algorithms/cpp/solutions/p0226_invert_binary_tree.hpp)
- 本地测试：[查看测试源码](../../../algorithms/cpp/tests/test_p0226.cpp)

## 从直觉到算法

可以另造一棵镜像树，但会增加 n 个节点并改变所有地址。本题允许修改输入，因而每个节点只需交换两条边，再让两棵子树做同样的工作。不必比较节点值，也不需要先收集遍历结果。

## 核心不变量与正确性

翻转空树显然正确。对非空节点先交换左右孩子，再把交换后的两棵子树各自翻转，得到 `mirror(T) = node(value, mirror(R), mirror(L))`。递归不会新建节点，所以节点身份不变；每个节点交换一次，整棵树成为镜像。交换和对子树递归的先后也可换成后序，只要每棵子树恰好处理一次。

## 操作步骤

1. 根为空时直接返回。
2. 用 `std::swap` 交换根的两条孩子指针。
3. 分别翻转现在的左子树和右子树。
4. 返回根。这里的返回值方便调用者保留入口，根地址实际上没有改变。

## 手工走一遍

根 7，左孩子 2（左孩子 1），右孩子 11（左孩子 9）。根交换后左边是 11、右边是 2；翻转 11 后 9 变成右孩子，翻转 2 后 1 也变成右孩子。再翻转一次会恢复原有连接，这是很有用的性质测试。

## C++17 完整实现

以下代码与仓库头文件内容一致。在线提交时平台通常已定义 `TreeNode`；本地通过 support.hpp 提供节点类型及标准库。命名空间用于同时编译多个题目的 Solution，不是算法要求。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0226 {
class Solution {
public:
    TreeNode* invertTree(TreeNode* root) {
        if (root == nullptr) return nullptr;
        std::swap(root->left, root->right);
        invertTree(root->left);
        invertTree(root->right);
        return root;
    }
};
} // namespace hot100::p0226
```

## 复杂度拆解

时间 O(n)。非递归额外数据 O(1)，调用栈 O(h)。不创建输出节点，返回指针占 O(1)；输入树原有 n 个节点不计为辅助空间。

## C++ 工程与边界陷阱

- 输入树会被修改，任何保存了左右孩子关系的外部视图都需要更新。
- `swap` 只交换地址，不会复制或析构节点；Arena 仍拥有原来的节点。
- 不要在交换后只递归原先一侧，或者对同一侧递归两次。
- 若结构是共享子树的 DAG 或有环，它已不满足普通二叉树契约，本算法可能重复修改或无限递归。

## 实现复用与边界检查

临时指针交换与 std::swap 都可用于反转左右子树。接口原地修改连接，不转移节点所有权，递归空间应按树高计算。

## 面试追问

- 不能修改原树时怎么做？递归分配新节点，同时把原右子树复制到新左边；要明确新树的释放责任。
- 为什么翻转两次一定恢复？每个节点执行两次交换，交换运算的平方是恒等变换。
- 若树很深，如何避免递归？用队列或显式栈逐个交换，队列空间取决于层宽。

## 两道练习与具体提示

1. 写一个非递归版本。提示：每次取出节点先交换孩子，再把两个非空孩子放入容器。
2. 实现 `mirrorCopy`，保持原树不动。提示：新节点的左孩子来自原右子树；测试前后原节点地址及连接均未改变。

## 本地验证

在仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp-build
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0226.cpp -o /tmp/hot100-cpp-build/p0226_invert_binary_tree
/tmp/hot100-cpp-build/p0226_invert_binary_tree
```

内存与未定义行为检查（编译器支持时）：

```bash
g++ -std=c++17 -Wall -Wextra -pedantic -g -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0226.cpp -o /tmp/hot100-cpp-build/p0226_invert_binary_tree_san
ASAN_OPTIONS=detect_leaks=1 /tmp/hot100-cpp-build/p0226_invert_binary_tree_san
```

测试用 assert 验证正常场景、边界与对象复用；源码中的固定随机种子使差分或性质检查可复现。输入节点由 TreeArena 管理；动态新建的输出树由 destroyTree 释放，二者的释放方式不可混用。测试过程不联网，也不作在线 AC 声明。
