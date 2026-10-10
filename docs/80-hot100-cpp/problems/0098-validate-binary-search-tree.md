# 98 验证二叉搜索树

> 热题 100 · 二叉树 · C++17 · Medium
> 训练目标：从子树状态、遍历顺序和节点所有权出发，写出可重复调用、可本地验证的解法。

## 题目契约

判断每个节点是否满足：整个左子树所有值严格小于自己，整个右子树所有值严格大于自己，且两侧也各自满足同一规则。重复值不合法；空树返回真。只读输入。

- 官方题目：[力扣 98 验证二叉搜索树](https://leetcode.cn/problems/validate-binary-search-tree/)
- 完整实现：[下载或查看头文件](../../../algorithms/cpp/solutions/p0098_validate_binary_search_tree.hpp)
- 本地测试：[查看测试源码](../../../algorithms/cpp/tests/test_p0098.cpp)

## 从直觉到算法

只比较节点与两个直接孩子会漏掉跨层约束。例如根 10，右孩子 15，而 15 的左孩子为 6，局部比较都成立，6 却违反了根的限制。可以向下传递允许的开区间，也可以利用 BST 中序序列严格递增的等价条件。我们用显式栈中序扫描，边访问边比较，无需保存全部 n 个值。

## 核心不变量与正确性

若树是严格 BST，中序先访问所有较小的左侧值，再访问根，再访问所有较大的右侧值，因此序列严格递增。反过来，若完整中序序列严格递增，任何节点前面的左子树值都小于它、后面的右子树值都大于它；同一性质也对每个子树的连续片段成立。所以中序严格递增是充分且必要条件。

## 操作步骤

1. 用栈和游标找到下一中序节点。
2. 用 `optional<int>` 表示“上一值可能还不存在”，而不选一个整数当哨兵。
3. 若上一值存在且当前值小于等于它，返回假。
4. 否则更新上一值，转向右子树；访问完毕返回真。

## 手工走一遍

根 10，左孩子 4，右孩子 15，15 的左孩子 6。中序访问为 4、10、6、15。访问 4 时无前驱；访问 10 时 `10>4`；访问 6 时 `6<=10`，立即失败。即使 6 小于其直接父节点 15，也不能挽救跨越祖先的违例。

## C++17 完整实现

以下代码与仓库头文件内容一致。在线提交时平台通常已定义 `TreeNode`；本地通过 support.hpp 提供节点类型及标准库。命名空间用于同时编译多个题目的 Solution，不是算法要求。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0098 {
class Solution {
public:
    bool isValidBST(TreeNode* root) {
        std::vector<TreeNode*> pending;
        std::optional<int> previous;
        TreeNode* cursor = root;
        while (cursor != nullptr || !pending.empty()) {
            while (cursor != nullptr) {
                pending.push_back(cursor);
                cursor = cursor->left;
            }
            cursor = pending.back();
            pending.pop_back();
            if (previous.has_value() && cursor->val <= *previous) return false;
            previous = cursor->val;
            cursor = cursor->right;
        }
        return true;
    }
};
} // namespace hot100::p0098
```

## 复杂度拆解

最坏时间 O(n)，首次逆序或重复可提前停止。显式栈 O(h) 辅助空间，递归栈 O(1)；`previous` 是 O(1)，布尔输出 O(1)。不构造完整中序数组。

## C++ 工程与边界陷阱

- `optional` 区分缺省与 `INT_MIN`，无需假定存在比所有 int 更小的 int。
- 严格 BST 要检查 `<=`，若只用 `<` 会接受重复值。
- 仅借用节点指针，不改树，也不释放任何节点。
- C++ 的 `long` 在部分 64 位平台仍是 32 位，不应据“64位系统”推断其能容纳 int 外侧边界。

## 实现复用与边界检查

LONG_MIN/LONG_MAX 配合 long 不保证能表示所有 int 边界外的哨兵。LLP64 平台上 long 与 int 可同为32位；使用可缺省前驱值可避开极值哨兵问题。

## 面试追问

- 若允许重复值只在右边，中序非降序足够吗？不够，还需区分重复值位于哪一侧，应采用带闭开语义的范围约束。
- 有环或共享节点怎么办？普通树契约不涵盖这些结构，需要另加访问标记或结构校验。
- 中序扫描怎样实现提前返回而不改变树？显式栈只读即可，Morris 则需考虑提前退出时恢复线索。

## 两道练习与具体提示

1. 写范围递归版本。提示：用 `long long` 边界和严格开区间；根值更新一侧边界。
2. 支持“右子树可重复、左子树仍严格小于”。提示：给每个边界加上是否允许等号的标记，不能只检查整体排序。

## 本地验证

在仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp-build
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0098.cpp -o /tmp/hot100-cpp-build/p0098_validate_binary_search_tree
/tmp/hot100-cpp-build/p0098_validate_binary_search_tree
```

内存与未定义行为检查（编译器支持时）：

```bash
g++ -std=c++17 -Wall -Wextra -pedantic -g -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0098.cpp -o /tmp/hot100-cpp-build/p0098_validate_binary_search_tree_san
ASAN_OPTIONS=detect_leaks=1 /tmp/hot100-cpp-build/p0098_validate_binary_search_tree_san
```

测试用 assert 验证正常场景、边界与对象复用；源码中的固定随机种子使差分或性质检查可复现。输入节点由 TreeArena 管理；动态新建的输出树由 destroyTree 释放，二者的释放方式不可混用。测试过程不联网，也不作在线 AC 声明。
