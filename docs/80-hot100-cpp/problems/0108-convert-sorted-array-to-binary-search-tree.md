# 108 将有序数组转换为二叉搜索树

> 热题 100 · 二叉树 · C++17 · Easy
> 训练目标：从子树状态、遍历顺序和节点所有权出发，写出可重复调用、可本地验证的解法。

## 题目契约

给定严格递增的整数数组，创建一棵高度平衡的二叉搜索树并返回根。允许多种正确形状。空数组返回空指针；数组不修改，新树节点由调用者负责释放。

- 官方题目：[力扣 108 将有序数组转换为二叉搜索树](https://leetcode.cn/problems/convert-sorted-array-to-binary-search-tree/)
- 完整实现：[下载或查看头文件](../../../algorithms/cpp/solutions/p0108_convert_sorted_array_to_binary_search_tree.hpp)
- 本地测试：[查看测试源码](../../../algorithms/cpp/tests/test_p0108.cpp)

## 从直觉到算法

按顺序把元素插入普通 BST，会形成向右链，虽然排序条件成立，却不平衡。每次取区间中点为根，左半段构造左子树，右半段构造右子树，就能控制两侧规模。直接复制左右数组也可做，但会让每层都搬移元素；传递半开下标区间 `[begin,end)` 可消除这项重复工作。

## 核心不变量与正确性

区间内元素严格递增，所以中点左侧全部小于根、右侧全部大于根；递归保持两边各自的 BST 性质。中点使两侧元素个数最多相差一，而且对子区间持续使用同一规则，因此产生的子树高度最多相差一。可用区间长度归纳：长度 m 的构造高度为 `floor(log2(m))+1`（m>0），两侧长度差至多一，对应高度差也至多一。

## 操作步骤

1. 用 `[begin,end)` 表示需要转换的元素，无须构造临时数组。
2. 若区间为空，返回空指针。
3. 计算 `middle=begin+(end-begin)/2`，创建根节点。
4. 左侧递归 `[begin,middle)`，右侧递归 `[middle+1,end)`，把结果接到根上。

## 手工走一遍

数组 `[-8,-3,0,5,12,20]` 的中点下标是 3，根取 5。左区间 `[-8,-3,0]` 根取 -3，孩子为 -8、0；右区间 `[12,20]` 根取 20，左孩子 12。各节点的左右高度差均不超过一，中序遍历恰好还原原数组。

## C++17 完整实现

以下代码与仓库头文件内容一致。在线提交时平台通常已定义 `TreeNode`；本地通过 support.hpp 提供节点类型及标准库。命名空间用于同时编译多个题目的 Solution，不是算法要求。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0108 {
class Solution {
    static TreeNode* build(const std::vector<int>& values, std::size_t begin, std::size_t end) {
        if (begin == end) return nullptr;
        const std::size_t middle = begin + (end - begin) / 2;
        TreeNode* root = new TreeNode(values[middle]);
        root->left = build(values, begin, middle);
        root->right = build(values, middle + 1, end);
        return root;
    }
public:
    TreeNode* sortedArrayToBST(std::vector<int>& nums) {
        return build(nums, 0, nums.size());
    }
};
} // namespace hot100::p0108
```

## 复杂度拆解

时间 O(n)，每个元素成为一个节点，没有切片复制。除输出节点与调用栈外辅助空间 O(1)；递归栈 O(log(n+1))，n=0 时为 O(1)。新建树占 O(n) 输出空间，不能把这项隐去后称为“总空间 O(log n)”。

## C++ 工程与边界陷阱

- 使用 `begin+(end-begin)/2` 避免 `begin+end` 溢出；用 `size_t` 与容器下标一致。
- 输出使用 `new` 创建。测试调用 `testing::destroyTree`；不要把输出同时交给 Arena 再手动释放。
- 本实现按竞赛常见约定假设分配成功。生产级实现需用带整棵子树删除器的 RAII 或所有权节点处理中途分配异常。
- 若输入含重复值，严格 BST 定义就不满足；不能默默把它当成本题合法输入。

## 实现复用与边界检查

每层复制左右子数组，总复制时间可达 O(n log n)。下标范围可把构建时间降到 O(n)，输出节点的必要分配仍需计入结果空间。

## 面试追问

- 中点取左中位或右中位有区别吗？形状可能不同，但平衡和中序序列都正确。
- 有序链表怎么构造？先知道长度，再通过中序模拟建树，让链表游标顺序推进。
- 如果节点类型用 `unique_ptr`，如何表达孩子？把孩子也改成独占指针，使销毁和异常处理自动化。

## 两道练习与具体提示

1. 对输出树编写校验器，检查中序数组相等且每个节点平衡。提示：后序返回高度，并在任意高度差超过一时失败。
2. 始终取左中位作为根。提示：非空半开区间用 `begin+(end-begin-1)/2`，注意必须先排除空区间。

## 本地验证

在仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp-build
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0108.cpp -o /tmp/hot100-cpp-build/p0108_convert_sorted_array_to_binary_search_tree
/tmp/hot100-cpp-build/p0108_convert_sorted_array_to_binary_search_tree
```

内存与未定义行为检查（编译器支持时）：

```bash
g++ -std=c++17 -Wall -Wextra -pedantic -g -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0108.cpp -o /tmp/hot100-cpp-build/p0108_convert_sorted_array_to_binary_search_tree_san
ASAN_OPTIONS=detect_leaks=1 /tmp/hot100-cpp-build/p0108_convert_sorted_array_to_binary_search_tree_san
```

测试用 assert 验证正常场景、边界与对象复用；源码中的固定随机种子使差分或性质检查可复现。输入节点由 TreeArena 管理；动态新建的输出树由 destroyTree 释放，二者的释放方式不可混用。测试过程不联网，也不作在线 AC 声明。
