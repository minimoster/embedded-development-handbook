# 101 对称二叉树

> 热题 100 · 二叉树 · C++17 · Easy
> 训练目标：从子树状态、遍历顺序和节点所有权出发，写出可重复调用、可本地验证的解法。

## 题目契约

判断整棵树是否关于根的竖直轴镜像对称。需要同时匹配节点值和空位结构；空树也视为对称。比较过程不修改输入。

- 官方题目：[力扣 101 对称二叉树](https://leetcode.cn/problems/symmetric-tree/)
- 完整实现：[下载或查看头文件](../../../algorithms/cpp/solutions/p0101_symmetric_tree.hpp)
- 本地测试：[查看测试源码](../../../algorithms/cpp/tests/test_p0101.cpp)

## 从直觉到算法

把两边遍历成值序列再比较，会把空孩子的位置丢掉；例如一侧的 3 在左边、另一侧的 3 也在左边，值序列相同却不对称。正确比较单位是“一对镜像位置”。比较左边的左孩子时，必须配上右边的右孩子，另一组也交叉配对。

## 核心不变量与正确性

`mirror(a,b)` 表示两棵子树互为镜像。两者均空为真；仅一者空为假；都非空时必须根值相等，且 `mirror(a.left,b.right)` 和 `mirror(a.right,b.left)` 都真。这个条件直接穷尽了所有镜像位置，递归返回布尔值便不需要持续维护可变成员标记。

## 操作步骤

1. 入口若根为空，返回真；否则比较根的左右子树。
2. 在辅助函数里先处理空指针组合，避免解引用空指针。
3. 比较当前两个值，不同就立即返回假。
4. 用短路与同时验证外侧的一对与内侧的一对。

## 手工走一遍

根为 5，两边都为 2；左侧 2 的右孩子是 4，右侧 2 的左孩子也是 4。外侧比较 `(null,null)` 为真，内侧比较 `(4,4)` 为真，所以树对称。若把右侧 4 移到右孩子，外侧比较会变为 `(null,4)`，立即判假，即使节点值没有变化。

## C++17 完整实现

以下代码与仓库头文件内容一致。在线提交时平台通常已定义 `TreeNode`；本地通过 support.hpp 提供节点类型及标准库。命名空间用于同时编译多个题目的 Solution，不是算法要求。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0101 {
class Solution {
    static bool mirror(TreeNode* a, TreeNode* b) {
        if (a == nullptr || b == nullptr) return a == b;
        if (a->val != b->val) return false;
        return mirror(a->left, b->right) && mirror(a->right, b->left);
    }
public:
    bool isSymmetric(TreeNode* root) {
        return root == nullptr || mirror(root->left, root->right);
    }
};
} // namespace hot100::p0101
```

## 复杂度拆解

最坏时间 O(n)，发现不匹配可提前停止。非调用栈辅助空间 O(1)，递归栈 O(h)；输出一个布尔值占 O(1)。递归中的镜像节点对只借用原树指针。

## C++ 工程与边界陷阱

- 先判断空，再读 `val`，不要在入口直接写 `root->left` 而忽略空根。
- 同一个 Solution 对象可先检查不对称树，再检查对称树，结果不应依赖前一次。
- 重复值不能说明结构相同，要让空位参与比较。
- 若用队列迭代，一次取出的是一对指针，允许指针为空；不是两个普通 BFS 值数组。

## 实现复用与边界检查

入口先处理空根，避免直接访问 root->left/right。成员布尔状态若未在每次调用重置，会污染后续结果；此实现使用局部返回值。

## 面试追问

- 对称性与两棵树相等有什么不同？相等按左对左、右对右，对称按左对右、右对左。
- 可否用序列法？可以，但序列必须包含空位并选择镜像一致的遍历方向。
- 大树如何短路？值或空位不一致时立即返回，避免扫描无关子树。

## 两道练习与具体提示

1. 写迭代版镜像比较。提示：队列元素用 `pair<TreeNode*,TreeNode*>`，比较后推入两组交叉孩子。
2. 找出一个“左右中序值数组相同但不对称”的反例。提示：使用重复值，让两个相同值都落在各自左孩子位置。

## 本地验证

在仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp-build
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0101.cpp -o /tmp/hot100-cpp-build/p0101_symmetric_tree
/tmp/hot100-cpp-build/p0101_symmetric_tree
```

内存与未定义行为检查（编译器支持时）：

```bash
g++ -std=c++17 -Wall -Wextra -pedantic -g -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0101.cpp -o /tmp/hot100-cpp-build/p0101_symmetric_tree_san
ASAN_OPTIONS=detect_leaks=1 /tmp/hot100-cpp-build/p0101_symmetric_tree_san
```

测试用 assert 验证正常场景、边界与对象复用；源码中的固定随机种子使差分或性质检查可复现。输入节点由 TreeArena 管理；动态新建的输出树由 destroyTree 释放，二者的释放方式不可混用。测试过程不联网，也不作在线 AC 声明。
