# 230 二叉搜索树中第 K 小的元素

> 热题 100 · 二叉树 · C++17 · Medium
> 训练目标：从子树状态、遍历顺序和节点所有权出发，写出可重复调用、可本地验证的解法。

## 题目契约

输入严格 BST 与合法序号 k（从 1 开始），返回第 k 小值。官方有效输入满足 `1<=k<=n`。本地实现额外对 k<=0 抛 `invalid_argument`，对空树或 k>n 抛 `out_of_range`，避免用合法整数值表示失败。

- 官方题目：[力扣 230 二叉搜索树中第 K 小的元素](https://leetcode.cn/problems/kth-smallest-element-in-a-bst/)
- 完整实现：[下载或查看头文件](../../../algorithms/cpp/solutions/p0230_kth_smallest_element_in_a_bst.hpp)
- 本地测试：[查看测试源码](../../../algorithms/cpp/tests/test_p0230.cpp)

## 从直觉到算法

收集所有值再排序可求第 k 小，但 BST 自带排序结构，完整排序是多余工作。即便先做中序再下标读取，也会保存 n 个值。中序扫描时每弹出一个节点就把 k 减一，减到零立刻返回，只保留通往下一节点的栈。

## 核心不变量与正确性

严格 BST 的中序节点值按升序排列。在已弹出 j 个节点后，原序号对应的剩余计数为 `k_original-j`，这些节点恰好是最小的 j 个。当计数为零时，当前节点就是排序序列中的第 k 项；提前停止不会影响答案，因为后续值只会更大。

## 操作步骤

1. 首先拒绝非正 k。
2. 沿左脊柱入栈，找到下一项中序节点。
3. 弹出后执行 `--k`，若为零便返回当前值。
4. 转向右子树继续。如果耗尽树仍未命中，说明序号超出节点数，抛出异常。

## 手工走一遍

根 8，左子树根 3（孩子 1、5），右孩子 12。找 k=3：先压 8、3、1；弹出 1 后 k=2，弹出 3 后 k=1，转向 5，弹出 5 后 k=0，返回 5。无需访问 8 的右子树。

## C++17 完整实现

以下代码与仓库头文件内容一致。在线提交时平台通常已定义 `TreeNode`；本地通过 support.hpp 提供节点类型及标准库。命名空间用于同时编译多个题目的 Solution，不是算法要求。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0230 {
class Solution {
public:
    int kthSmallest(TreeNode* root, int k) {
        if (k <= 0) throw std::invalid_argument("k must be positive");
        std::vector<TreeNode*> pending;
        TreeNode* cursor = root;
        while (cursor != nullptr || !pending.empty()) {
            while (cursor != nullptr) {
                pending.push_back(cursor);
                cursor = cursor->left;
            }
            cursor = pending.back();
            pending.pop_back();
            if (--k == 0) return cursor->val;
            cursor = cursor->right;
        }
        throw std::out_of_range("k exceeds the number of nodes");
    }
};
} // namespace hot100::p0230
```

## 复杂度拆解

寻找前 k 项前需要沿树高下降，时间 O(h+k)，最坏 O(n)。显式栈 O(h)，无递归所以调用栈 O(1)；返回整数输出空间 O(1)。相比完整中序数组，省去 O(n) 存储，并允许提前结束。

## C++ 工程与边界陷阱

- 使用局部 k 与局部栈，无残留序号或答案，支持复用 Solution。
- 不用 -1 作“尚未找到”标记，因通用 BST 值可能恰好是 -1。
- API 抛异常只用于额外防御边界，合法在线输入不会经过异常分支。
- 栈持有借用地址；不能把返回值的查找过程当作节点所有权转移。

## 实现复用与边界检查

成员答案与计数器若未重置，会污染不同 k 的查询。-1 哨兵未必适用于任意 int 树；此实现采用局部计数与直接返回，非法序号有明确错误通道。

## 面试追问

- 若同一棵树要查询很多次，怎么办？给节点维护子树大小，按左侧大小定位，单次 O(h)。
- 如果树不断插入删除？需要带计数的平衡树，并在旋转后更新子树大小。
- 若允许重复值，k 统计不同值还是节点？先澄清定义，否则跳过重复与不跳过会给出不同答案。

## 两道练习与具体提示

1. 找第 k 大值。提示：改为右、根、左的逆中序，仍对弹出节点计数。
2. 给 BST 添加子树节点数并实现选择查询。提示：设左侧大小为 s，k=s+1 命中根，k<=s 去左侧，否则带 k-s-1 去右侧。

## 本地验证

在仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp-build
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0230.cpp -o /tmp/hot100-cpp-build/p0230_kth_smallest_element_in_a_bst
/tmp/hot100-cpp-build/p0230_kth_smallest_element_in_a_bst
```

内存与未定义行为检查（编译器支持时）：

```bash
g++ -std=c++17 -Wall -Wextra -pedantic -g -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0230.cpp -o /tmp/hot100-cpp-build/p0230_kth_smallest_element_in_a_bst_san
ASAN_OPTIONS=detect_leaks=1 /tmp/hot100-cpp-build/p0230_kth_smallest_element_in_a_bst_san
```

测试用 assert 验证正常场景、边界与对象复用；源码中的固定随机种子使差分或性质检查可复现。输入节点由 TreeArena 管理；动态新建的输出树由 destroyTree 释放，二者的释放方式不可混用。测试过程不联网，也不作在线 AC 声明。
