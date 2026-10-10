# 104 二叉树的最大深度

> 热题 100 · 二叉树 · C++17 · Easy
> 训练目标：从子树状态、遍历顺序和节点所有权出发，写出可重复调用、可本地验证的解法。

## 题目契约

返回根到最远叶子的路径上的节点数。空树深度为 0，只有根的树深度为 1；这里计的是节点，不是边。函数不改变输入树。

- 官方题目：[力扣 104 二叉树的最大深度](https://leetcode.cn/problems/maximum-depth-of-binary-tree/)
- 完整实现：[下载或查看头文件](../../../algorithms/cpp/solutions/p0104_maximum_depth_of_binary_tree.hpp)
- 本地测试：[查看测试源码](../../../algorithms/cpp/tests/test_p0104.cpp)

## 从直觉到算法

逐个叶子求它到根的距离，若没有父指针，就容易重复搜索祖先。更自然的是让每个子树直接告诉父节点自己的高度：父节点只需取左右高度的较大值再加一。这样结果由局部答案组合，完全不需要存一个跨调用的“当前最大深度”。

## 核心不变量与正确性

定义 `depth(T)` 为子树 T 的节点高度。空树为 0；非空树的一条根到叶路径必先选择左或右孩子，所以最长长度正好是 `1 + max(depth(L), depth(R))`。对子树大小作归纳：两边返回值正确时，父节点的公式必然正确。

## 操作步骤

1. 遇到空指针直接返回 0。
2. 递归求出左子树高度。
3. 递归求出右子树高度。
4. 返回较大高度加一。所有变量都是当前调用的局部变量，下一次调用自然重新计算。

## 手工走一遍

根 8 左边是叶子 3，右边为 10，10 的右孩子为 12。叶子 3 和 12 各返回 1；10 得到 `max(0,1)+1=2`；8 得到 `max(1,2)+1=3`。若接着用同一个对象计算叶子 3，答案仍是 1，不会继承先前的 3。

## C++17 完整实现

以下代码与仓库头文件内容一致。在线提交时平台通常已定义 `TreeNode`；本地通过 support.hpp 提供节点类型及标准库。命名空间用于同时编译多个题目的 Solution，不是算法要求。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0104 {
class Solution {
public:
    int maxDepth(TreeNode* root) {
        if (root == nullptr) return 0;
        const int left = maxDepth(root->left);
        const int right = maxDepth(root->right);
        return 1 + std::max(left, right);
    }
};
} // namespace hot100::p0104
```

## 复杂度拆解

时间 O(n)，每个节点求一次高度。除递归调用栈外辅助空间 O(1)；调用栈 O(h)，其中 h 是树高，平衡树 O(log n)，链状树 O(n)。返回单个整数，输出空间 O(1)。

## C++ 工程与边界陷阱

- 参数是借用指针，不分配或释放节点。
- 不把层数保存在未重置的成员里，否则多次调用可能读到旧答案。
- `int` 深度对题目规模足够；若通用接口支持超过 `INT_MAX` 个层级，应改变返回类型。
- 深链可能让递归栈耗尽，工程代码可以改用队列计层数。

## 实现复用与边界检查

可复用求深度接口应避免保留上次树的最大深度。先计算深树再计算单节点树或空树，是检查跨调用状态的有效回归用例。

## 面试追问

- 最大深度和最小深度为何不能只把 `max` 换成 `min`？单侧孩子为空时，空路径不是一条到叶子的有效路径。
- BFS 的空间是什么？队列占 O(w)，w 为最大层宽；DFS 占 O(h)，应按树形选择。
- 若节点持续插入，能否增量维护高度？有父指针时可向上更新，必要时到首次不变的祖先停止。

## 两道练习与具体提示

1. 实现最小深度。提示：只有一侧非空时必须沿非空侧走；两侧都非空才取最小值。
2. 用 BFS 返回最大深度。提示：进入一层前固定队列长度，处理完这批节点后把层数加一。

## 本地验证

在仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp-build
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0104.cpp -o /tmp/hot100-cpp-build/p0104_maximum_depth_of_binary_tree
/tmp/hot100-cpp-build/p0104_maximum_depth_of_binary_tree
```

内存与未定义行为检查（编译器支持时）：

```bash
g++ -std=c++17 -Wall -Wextra -pedantic -g -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0104.cpp -o /tmp/hot100-cpp-build/p0104_maximum_depth_of_binary_tree_san
ASAN_OPTIONS=detect_leaks=1 /tmp/hot100-cpp-build/p0104_maximum_depth_of_binary_tree_san
```

测试用 assert 验证正常场景、边界与对象复用；源码中的固定随机种子使差分或性质检查可复现。输入节点由 TreeArena 管理；动态新建的输出树由 destroyTree 释放，二者的释放方式不可混用。测试过程不联网，也不作在线 AC 声明。
