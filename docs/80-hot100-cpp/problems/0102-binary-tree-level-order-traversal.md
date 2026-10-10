# 102 二叉树的层序遍历

> 热题 100 · 二叉树 · C++17 · Medium
> 训练目标：从子树状态、遍历顺序和节点所有权出发，写出可重复调用、可本地验证的解法。

## 题目契约

按深度从浅到深输出二维数组，每个内层数组是一层，层内从左到右。空树返回空二维数组。节点值只读，树连接不变。

- 官方题目：[力扣 102 二叉树的层序遍历](https://leetcode.cn/problems/binary-tree-level-order-traversal/)
- 完整实现：[下载或查看头文件](../../../algorithms/cpp/solutions/p0102_binary_tree_level_order_traversal.hpp)
- 本地测试：[查看测试源码](../../../algorithms/cpp/tests/test_p0102.cpp)

## 从直觉到算法

DFS 也能把值写入 `answer[depth]`，但需要传递深度并按访问顺序维护各层。队列天然让近处节点先于远处节点处理。要额外解决的是“当前层在哪结束”：在处理这一层之前固定队列长度，随后加入的孩子属于下一层，不参与本轮。

## 核心不变量与正确性

每轮开始时，队列恰好包含某一层从左到右的节点。取出这一层固定数量的节点时，按左、右顺序追加孩子，于是剩余队列恰好是下一层的从左到右顺序。根构成第一层，所以归纳得到所有层都正确。

## 操作步骤

1. 空根直接返回空结果，否则入队。
2. 每轮取 `count=q.size()`，建立一个新的层数组并预留 count 个位置。
3. 恰好弹出 count 个节点，记录值，依次加入非空左、右孩子。
4. 将完成的层移动到结果中，继续直到队列为空。

## 手工走一遍

根 4，左孩子 1，右孩子 7；1 的右孩子 2，7 的左孩子 6。第一轮只处理 4，得到 `[4]` 并形成队列 `[1,7]`。第二轮固定长度 2，虽在处理中加入 2、6，它们不会提前被处理；最终得到 `[[4],[1,7],[2,6]]`。

## C++17 完整实现

以下代码与仓库头文件内容一致。在线提交时平台通常已定义 `TreeNode`；本地通过 support.hpp 提供节点类型及标准库。命名空间用于同时编译多个题目的 Solution，不是算法要求。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0102 {
class Solution {
public:
    std::vector<std::vector<int>> levelOrder(TreeNode* root) {
        std::vector<std::vector<int>> answer;
        if (root == nullptr) return answer;
        std::queue<TreeNode*> pending;
        pending.push(root);
        while (!pending.empty()) {
            const std::size_t count = pending.size();
            std::vector<int> level;
            level.reserve(count);
            for (std::size_t i = 0; i < count; ++i) {
                TreeNode* node = pending.front();
                pending.pop();
                level.push_back(node->val);
                if (node->left != nullptr) pending.push(node->left);
                if (node->right != nullptr) pending.push(node->right);
            }
            answer.push_back(std::move(level));
        }
        return answer;
    }
};
} // namespace hot100::p0102
```

## 复杂度拆解

时间 O(n)。辅助队列 O(w)，w 是最大层宽；处理中可能同时保留当前层尾部和下一层前部，仍为 O(w)。临时层数组 O(w)，移动后成为输出的一部分。无递归，栈 O(1)；输出含 n 个值与 h 个内层数组头部，为 O(n+h)=O(n)。

## C++ 工程与边界陷阱

- `q.size()` 返回 `size_t`，循环计数也用 `size_t`，避免不必要的有符号/无符号混用。
- 不能把循环上限写成每次变化的 `q.size()`，因为孩子入队会改变层边界。
- 队列只保存借用地址，不管理节点生命期。
- `std::move(level)` 转移数组缓冲区而不修改树；此后不再读取被移动的局部数组。

## 实现复用与边界检查

每轮开始固定当前层的队列长度，才能分离本层与下一层。此实现采用 size_t，独立构建层数组，空间分析包含队列混合相邻两层的情况。

## 面试追问

- 怎样做锯齿形层序？保留入队顺序，每层填值时按层号决定正向或反向索引。
- 如何把每层结果流式发送给消费者？一层处理完成便回调，不必保存 O(n) 结果。
- BFS 与递归 DFS 的峰值空间如何比较？宽树 BFS 较大，深链 DFS 调用栈较大。

## 两道练习与具体提示

1. 返回每层最大值。提示：层边界不变，仅把临时数组改成一项最大值。
2. 返回从底到顶的层序。提示：先正常 BFS 再翻转外层数组，避免每次在数组头部插入导致搬移。

## 本地验证

在仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp-build
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0102.cpp -o /tmp/hot100-cpp-build/p0102_binary_tree_level_order_traversal
/tmp/hot100-cpp-build/p0102_binary_tree_level_order_traversal
```

内存与未定义行为检查（编译器支持时）：

```bash
g++ -std=c++17 -Wall -Wextra -pedantic -g -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0102.cpp -o /tmp/hot100-cpp-build/p0102_binary_tree_level_order_traversal_san
ASAN_OPTIONS=detect_leaks=1 /tmp/hot100-cpp-build/p0102_binary_tree_level_order_traversal_san
```

测试用 assert 验证正常场景、边界与对象复用；源码中的固定随机种子使差分或性质检查可复现。输入节点由 TreeArena 管理；动态新建的输出树由 destroyTree 释放，二者的释放方式不可混用。测试过程不联网，也不作在线 AC 声明。
