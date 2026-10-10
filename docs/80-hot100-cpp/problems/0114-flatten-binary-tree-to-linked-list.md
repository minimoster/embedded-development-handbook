# 114 二叉树展开为链表

> 热题 100 · 二叉树 · C++17 · Medium
> 训练目标：从子树状态、遍历顺序和节点所有权出发，写出可重复调用、可本地验证的解法。

## 题目契约

原地将树变成只通过 right 连接的链，所有 left 必须为空；链的节点顺序是原树前序。必须保留原有节点身份，不分配新节点。空树无需操作。

- 官方题目：[力扣 114 二叉树展开为链表](https://leetcode.cn/problems/flatten-binary-tree-to-linked-list/)
- 完整实现：[下载或查看头文件](../../../algorithms/cpp/solutions/p0114_flatten_binary_tree_to_linked_list.hpp)
- 本地测试：[查看测试源码](../../../algorithms/cpp/tests/test_p0114.cpp)

## 从直觉到算法

先保存前序节点数组，再逐个连接是清晰的 O(n) 辅助空间基线；反向前序递归连接可降到 O(h) 栈。本课进一步利用指针拼接：若当前节点有左子树，把原右子树挂到左子树“沿右孩子走到底”的节点右边，再将整棵左子树搬到当前右边。继续沿 right 处理，可以只用常数个指针。

## 核心不变量与正确性

处理当前节点前，已经过的前缀已成为正确前序链且 left 为空，剩余节点仍按待展开的子树连接。前序要求当前节点之后先遍历左子树，再遍历原右子树。左子树最右链末端的 right 原本为空，把原右子树接到那里，会把它延后到整个左子树处理完以后；如果该末端还有左子树，后续拼接仍会把这个左子树插在原右子树之前。因此它不必是原前序的最后一个节点。搬移并清空当前 left 后，前缀延长一项且顺序不变。

## 操作步骤

1. 游标从根开始，沿 right 逐步前进。
2. 若当前有左孩子，令 `join` 为左孩子，沿它的 right 找到空右指针。
3. 将原右子树接到 `join->right`；再将左子树接到当前 right。
4. 清空当前 left，然后继续走当前 right。没有左孩子时直接前进。

## 手工走一遍

根 8 的左孩子 3、右孩子 10；3 的右孩子为 6，6 的左孩子为 4。先把 10 挂到 6 的 right，再把 3 移到 8 的 right。走到 6 时，其左孩子 4 仍需插到原 right=10 前面，变成 `6→4→10`。最终链是 `8→3→6→4→10`，对应原前序。这也说明首次找到的 6 不是原左子树前序最后一项。

## C++17 完整实现

以下代码与仓库头文件内容一致。在线提交时平台通常已定义 `TreeNode`；本地通过 support.hpp 提供节点类型及标准库。命名空间用于同时编译多个题目的 Solution，不是算法要求。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0114 {
class Solution {
public:
    void flatten(TreeNode* root) {
        for (TreeNode* current = root; current != nullptr; current = current->right) {
            if (current->left == nullptr) continue;
            TreeNode* join = current->left;
            while (join->right != nullptr) join = join->right;
            join->right = current->right;
            current->right = current->left;
            current->left = nullptr;
        }
    }
};
} // namespace hot100::p0114
```

## 复杂度拆解

时间 O(n)。外层每个节点处理一次；寻找拼接点只扫描当前未搬移左子树的原右脊柱。此前新增的“连接原右子树”的边不会成为以后某次左子树搜索的内部边，因此原右边在这些搜索中最多被扫描一次，合计线性，而不是每轮都扫描整棵树。辅助空间 O(1)，无递归栈，新增输出空间 O(1)，节点本身原地复用。

## C++ 工程与边界陷阱

- 拼接顺序不能先清空 left，否则左子树入口丢失。
- `join` 是结构上的右链末端，不能错误地声称它总是前序最后的叶子。
- 不分配哑节点，不 `delete` 原节点。Arena 仍按各节点地址独立释放。
- 本操作破坏原树结构，后续若要原树必须在操作前备份；多次 flatten 已展开的链是安全的。

## 实现复用与边界检查

不必要的堆哑节点需要回收，前驱指针必须初始化后再读取。此实现采用无新分配、无跨调用成员的原地拼接版本。

## 面试追问

- 能否在 O(1) 空间下恢复原树？一般不行，左右分支信息已经丢失，必须事先保存结构信息。
- 递归方案怎样避免成员状态污染？把前一个节点作为入口局部指针，以引用传入反向前序辅助函数。
- 为什么测试要比较指针序列，而不只比较值？重复值会掩盖节点遗漏、重复或新建节点的问题。

## 两道练习与具体提示

1. 写 O(n) 数组基线。提示：先收集原节点地址，再设置每个 left=null、right=下一地址，末项 right=null。
2. 用右、左、根的递归顺序重写。提示：入口令 previous=null；每个节点接到 previous，最后更新 previous=当前节点。

## 本地验证

在仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp-build
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0114.cpp -o /tmp/hot100-cpp-build/p0114_flatten_binary_tree_to_linked_list
/tmp/hot100-cpp-build/p0114_flatten_binary_tree_to_linked_list
```

内存与未定义行为检查（编译器支持时）：

```bash
g++ -std=c++17 -Wall -Wextra -pedantic -g -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0114.cpp -o /tmp/hot100-cpp-build/p0114_flatten_binary_tree_to_linked_list_san
ASAN_OPTIONS=detect_leaks=1 /tmp/hot100-cpp-build/p0114_flatten_binary_tree_to_linked_list_san
```

测试用 assert 验证正常场景、边界与对象复用；源码中的固定随机种子使差分或性质检查可复现。输入节点由 TreeArena 管理；动态新建的输出树由 destroyTree 释放，二者的释放方式不可混用。测试过程不联网，也不作在线 AC 声明。
