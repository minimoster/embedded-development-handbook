# 105 从前序与中序遍历序列构造二叉树

> 热题 100 · 二叉树 · C++17 · Medium
> 训练目标：从子树状态、遍历顺序和节点所有权出发，写出可重复调用、可本地验证的解法。

## 题目契约

给出同一棵树的前序数组与中序数组，节点值互不相同，长度一致且遍历合法，构造并返回新树。空数组对应空树。输入数组保持不变，输出新节点由调用者释放。非法遍历不属于此实现契约。

- 官方题目：[力扣 105 从前序与中序遍历序列构造二叉树](https://leetcode.cn/problems/construct-binary-tree-from-preorder-and-inorder-traversal/)
- 完整实现：[下载或查看头文件](../../../algorithms/cpp/solutions/p0105_construct_binary_tree_from_preorder_and_inorder_traversal.hpp)
- 本地测试：[查看测试源码](../../../algorithms/cpp/tests/test_p0105.cpp)

## 从直觉到算法

前序第一个值是根，在中序中找到根便能分割左、右子树。基线每次线性搜索根并复制子数组，偏斜树会达到 O(n²)。提前建立值到中序下标的哈希表，递归只传中序区间，让一个前序游标按根、左、右的顺序前进，就不再复制或重新搜索。

## 核心不变量与正确性

`build(begin,end)` 构造中序半开区间 `[begin,end)` 对应的子树；进入时 next 指向这棵子树的前序首项。读取该项为根，唯一中序位置 middle 切出左、右区间。先完成左边会恰好消耗左子树节点数个前序项，再完成右边；返回后 next 指向整个区间之后的下一棵待构造子树。合法且无重复的遍历保证根位置总在当前区间内，因此划分唯一。

## 操作步骤

1. 扫描 inorder 建立 `position[value]=index`。
2. 在入口创建 `next=0`，确保对象复用也从前序开头读。
3. 递归遇到空区间返回空指针，否则读 preorder[next++] 创建根。
4. 根据根的中序位置，先构造左区间，再构造右区间并连接。

## 手工走一遍

前序 `[7,3,1,5,11,9]`，中序 `[1,3,5,7,9,11]`。先读 7，位置 3，把中序分成 `[1,3,5]` 与 `[9,11]`。左边下一个前序值 3，位置 1，建立孩子 1、5；随后 next 来到 11，位置 5，左侧建 9。重建后两份遍历与输入一一相同。

## C++17 完整实现

以下代码与仓库头文件内容一致。在线提交时平台通常已定义 `TreeNode`；本地通过 support.hpp 提供节点类型及标准库。命名空间用于同时编译多个题目的 Solution，不是算法要求。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0105 {
class Solution {
    static TreeNode* build(const std::vector<int>& preorder,
                           const std::unordered_map<int, std::size_t>& position,
                           std::size_t begin, std::size_t end, std::size_t& next) {
        if (begin == end) return nullptr;
        const int value = preorder[next++];
        const std::size_t middle = position.at(value);
        TreeNode* root = new TreeNode(value);
        root->left = build(preorder, position, begin, middle, next);
        root->right = build(preorder, position, middle + 1, end, next);
        return root;
    }
public:
    TreeNode* buildTree(std::vector<int>& preorder, std::vector<int>& inorder) {
        std::unordered_map<int, std::size_t> position;
        position.reserve(inorder.size());
        for (std::size_t i = 0; i < inorder.size(); ++i) position.emplace(inorder[i], i);
        std::size_t next = 0;
        return build(preorder, position, 0, inorder.size(), next);
    }
};
} // namespace hot100::p0105
```

## 复杂度拆解

哈希表平均查找 O(1) 时，总时间期望 O(n)；在恶劣哈希碰撞模型下可达 O(n²)，不应把哈希最坏时间写成恒定。索引表 O(n) 辅助空间，递归栈 O(h)，输出新节点 O(n)。没有子数组切片；若希望确定性复杂度，可用有序映射，时间 O(n log n)。

## C++ 工程与边界陷阱

- next 是入口局部变量，通过引用推进，不能遗留在没有重置的成员里。
- 哈希表用 `at` 查根；契约外缺失值会抛异常，但本实现不提供完整非法输入验证或异常分配回滚。
- 构造节点来自 `new`，测试最后调用 destroyTree。输入树若由 Arena 管理，不能把新树误交给同一个手工释放路径。
- 重复值使 value→index 不再唯一，需要额外信息或不同重建目标，不能覆盖映射后继续宣称唯一。

## 实现复用与边界检查

每层线性找根并复制子数组，在偏斜树上可达 O(n²)。游标若为成员也应每次重置；此实现用局部游标、索引表与下标区间。

## 面试追问

- 只有前序和后序能唯一确定吗？一般不能，例如单孩子在左还是在右无法区分。
- 需要支持非法输入时应校验什么？长度、重复、值集合一致，以及每次根位置落在当前区间内；并用 RAII 防止失败途中泄漏。
- 怎样避免深树递归？可用栈同步消费前序与中序，在中序匹配时弹出已完成的祖先，再接右子树。

## 两道练习与具体提示

1. 用中序与后序重建。提示：从后序尾部取根，必须先递归右区间再递归左区间。
2. 实现不使用哈希的迭代重建。提示：栈保存还未与中序游标匹配的节点，前序新节点在未匹配时接左，匹配并弹栈后接右。

## 本地验证

在仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp-build
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0105.cpp -o /tmp/hot100-cpp-build/p0105_construct_binary_tree_from_preorder_and_inorder_traversal
/tmp/hot100-cpp-build/p0105_construct_binary_tree_from_preorder_and_inorder_traversal
```

内存与未定义行为检查（编译器支持时）：

```bash
g++ -std=c++17 -Wall -Wextra -pedantic -g -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0105.cpp -o /tmp/hot100-cpp-build/p0105_construct_binary_tree_from_preorder_and_inorder_traversal_san
ASAN_OPTIONS=detect_leaks=1 /tmp/hot100-cpp-build/p0105_construct_binary_tree_from_preorder_and_inorder_traversal_san
```

测试用 assert 验证正常场景、边界与对象复用；源码中的固定随机种子使差分或性质检查可复现。输入节点由 TreeArena 管理；动态新建的输出树由 destroyTree 释放，二者的释放方式不可混用。测试过程不联网，也不作在线 AC 声明。
