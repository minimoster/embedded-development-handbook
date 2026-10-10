# 199 二叉树的右视图

> 热题 100 · 二叉树 · C++17 · Medium
> 训练目标：从子树状态、遍历顺序和节点所有权出发，写出可重复调用、可本地验证的解法。

## 题目契约

按深度从上到下返回每层最靠右节点的值。不是沿着所有 right 指针走到底；右侧浅子树结束后，更深处的左侧节点也可能出现在右视图。空树返回空数组，输入只读。

- 官方题目：[力扣 199 二叉树的右视图](https://leetcode.cn/problems/binary-tree-right-side-view/)
- 完整实现：[下载或查看头文件](../../../algorithms/cpp/solutions/p0199_binary_tree_right_side_view.hpp)
- 本地测试：[查看测试源码](../../../algorithms/cpp/tests/test_p0199.cpp)

## 从直觉到算法

BFS 每层取最后一项很直观，需要维护层队列。本课采用先右后左的 DFS：某个深度第一次被访问的节点必是这一层最靠右的节点。结果数组的长度即可告诉我们该深度是否已经有答案，无须额外 visited 集合。

## 核心不变量与正确性

DFS 在任何节点都先处理整个右子树，再处理左子树。在固定深度上，同一祖先的右分支节点总先于左分支节点；递归地看，访问顺序就是该层从右到左。故首次到达深度 d 时记录的节点是最右节点。以后到达同一深度的节点只可能更靠左，不能覆盖该值。

## 操作步骤

1. 带深度参数从根的深度 0 开始。
2. 若节点为空，结束当前分支。
3. 当 `answer.size()==depth`，说明该层第一次出现，追加节点值。
4. 先递归右孩子，再递归左孩子。后续同深度节点不覆盖答案。

## 手工走一遍

根 6 的左孩子 2，右孩子 9；2 的右孩子为 4，9 没有孩子。先记录深度 0 的 6，再记录深度 1 的 9。访问 2 时深度 1 已有答案；但走到 4 时深度 2 首次出现，因此追加 4，结果 `[6,9,4]`。这说明只顺着右链得到 `[6,9]` 会漏层。

## C++17 完整实现

以下代码与仓库头文件内容一致。在线提交时平台通常已定义 `TreeNode`；本地通过 support.hpp 提供节点类型及标准库。命名空间用于同时编译多个题目的 Solution，不是算法要求。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0199 {
class Solution {
    static void visit(TreeNode* node, std::size_t depth, std::vector<int>& answer) {
        if (node == nullptr) return;
        if (answer.size() == depth) answer.push_back(node->val);
        visit(node->right, depth + 1, answer);
        visit(node->left, depth + 1, answer);
    }
public:
    std::vector<int> rightSideView(TreeNode* root) {
        std::vector<int> answer;
        visit(root, 0, answer);
        return answer;
    }
};
} // namespace hot100::p0199
```

## 复杂度拆解

时间 O(n)，因为即使一层已有答案，左子树仍可能提供更深层。除输出和栈外辅助空间 O(1)，递归栈 O(h)。输出数组有 h 项，占 O(h)。

## C++ 工程与边界陷阱

- 深度用 `size_t` 与数组长度直接比较。
- 顺序必须右先左；左先右却仍“首次记录”会变成左视图。
- 不修改树，也不持有节点的所有权。
- 若深链可能超过系统栈，改用 BFS 或显式 `(node,depth)` 栈；后者压栈顺序要与弹出顺序相反。

## 实现复用与边界检查

按左到右排列的本层队列，其最后节点就是该层右视图。左先右并覆盖答案或右先左只记录第一次也都可成立，关键是维护明确不变量。

## 面试追问

- BFS 如何实现同样的结果？固定一层长度，记录本轮最后弹出的节点。
- 为什么不能找到右孩子就忽略左子树？右子树可能高度不足，左子树的深节点仍可见。
- 若要求同时输出左右视图，怎样复用层遍历？每层取第一项与最后一项。

## 两道练习与具体提示

1. 返回左视图。提示：把 DFS 子树顺序改为左先右，仍只记录每层第一次。
2. 用显式栈重写右视图。提示：先压左孩子再压右孩子，LIFO 才会先处理右边，并记录每个节点的深度。

## 本地验证

在仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp-build
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0199.cpp -o /tmp/hot100-cpp-build/p0199_binary_tree_right_side_view
/tmp/hot100-cpp-build/p0199_binary_tree_right_side_view
```

内存与未定义行为检查（编译器支持时）：

```bash
g++ -std=c++17 -Wall -Wextra -pedantic -g -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0199.cpp -o /tmp/hot100-cpp-build/p0199_binary_tree_right_side_view_san
ASAN_OPTIONS=detect_leaks=1 /tmp/hot100-cpp-build/p0199_binary_tree_right_side_view_san
```

测试用 assert 验证正常场景、边界与对象复用；源码中的固定随机种子使差分或性质检查可复现。输入节点由 TreeArena 管理；动态新建的输出树由 destroyTree 释放，二者的释放方式不可混用。测试过程不联网，也不作在线 AC 声明。
