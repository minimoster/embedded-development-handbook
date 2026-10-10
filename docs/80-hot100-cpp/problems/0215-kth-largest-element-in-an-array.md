# 215. 数组中的第K个最大元素：C++17 推导与实现

> 分类：堆 · 难度：MEDIUM · 官方 Hot 100 顺序：74/100

## 题意与调用约定

返回排序后第k个最大值，重复元素占各自名次；k从1开始且在合法范围内。主实现原地重排数组，调用后不能依赖原顺序。

题面与最新约束见 [力扣中文官方题目](https://leetcode.cn/problems/kth-largest-element-in-an-array/) / [LeetCode 官方英文题目](https://leetcode.com/problems/kth-largest-element-in-an-array/)。下文给出独立编写的推导、示例与实现；测试是本地验证，没有声称在线评测已通过。

## 从直觉到优化

全排序 O(n log n) 做了许多无关工作；维护k个最大值的堆要 O(n log k)。选择问题只需保留含目标秩的一侧。随机 pivot 加三路划分能一次排除所有等于 pivot 的项，特别适合重复值多的输入。

## 不变量与正确性

三路扫描保持 [lo,less) 小于pivot、[less,scan) 等于pivot、(greater,hi] 大于pivot，[scan,greater] 未知。划分结束后目标升序秩 wanted=n-k 若在等值区间即可返回；否则只在相应一侧继续。被丢弃侧不可能含目标秩。

## 算法步骤

1. 把第k大换成升序零基秩 n-k。
2. 在当前区间均匀随机选 pivot 的值，复制值后再交换。
3. 通过 less/scan/greater 三指针原地划成小、等、大三段。
4. 目标位于等值段直接返回；否则缩至左段或右段。

## 手工追踪

[9,2,9,4,1] 找第2大，wanted=3。若pivot=4，划分得到小于4的[2,1]、等于4的[4]、大于4的[9,9]；秩3落在右段。下一轮pivot=9，整段等于9，返回9。重复值不是去重后排名。

## 完整 C++17 实现

以下内容来自本课 [可编译头文件](../../../algorithms/cpp/solutions/p0215_kth_largest_element_in_an_array.hpp)，namespace用于让100题可同时进入同一工程。在线提交时一般只保留评测要求的类声明，并按平台要求处理标准库命名；本地辅助头文件负责统一导入标准库。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0215 {
class Solution {
public:
    int findKthLargest(std::vector<int>& nums, int k) {
        int n = static_cast<int>(nums.size());
        if (k < 1 || k > n) throw std::invalid_argument("invalid k");
        int wanted = n - k, lo = 0, hi = n - 1;
        std::mt19937 rng(std::random_device{}());
        while (lo <= hi) {
            int pivot = nums[std::uniform_int_distribution<int>(lo, hi)(rng)];
            int less = lo, scan = lo, greater = hi;
            while (scan <= greater) {
                if (nums[scan] < pivot) std::swap(nums[less++], nums[scan++]);
                else if (nums[scan] > pivot) std::swap(nums[scan], nums[greater--]);
                else ++scan;
            }
            if (wanted < less) hi = less - 1;
            else if (wanted > greater) lo = greater + 1;
            else return pivot;
        }
        throw std::logic_error("unreachable selection state");
    }
};
} // namespace hot100::p0215
```

## 复杂度核算

均匀随机 pivot 下，期望时间 O(n)，最坏 O(n²)；三路划分对全相等输入只扫描一次。额外空间 O(1)，迭代形式没有递归栈；mt19937 状态大小是与n无关的常数。

## C++ 常见陷阱

- 把 pivot 保存为值，不能保存 nums 某项的引用，交换后引用内容可能改变。
- 交换 scan 与 greater 后不能 ++scan，因为换入项尚未分类。
- random_device 可能在受限平台不可用或抛异常；可注入随机引擎以改善可复现测试，但固定已知pivot序列不提供对任意对抗输入的随机期望保证。

## 测试与运行

[完整测试源文件](../../../algorithms/cpp/tests/test_p0215.cpp)：800组固定种子数组，对每个合法k与排序基线比较；含大量重复、全相等与整数极值，正确结果不依赖pivot随机路径。

从仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0215.cpp -o /tmp/hot100-cpp/test_p0215
/tmp/hot100-cpp/test_p0215
# 环境支持时增加运行期检查：
g++ -std=c++17 -Wall -Wextra -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0215.cpp -o /tmp/hot100-cpp/test_p0215_san
ASAN_OPTIONS=detect_leaks=0 /tmp/hot100-cpp/test_p0215_san
```

本次环境的LeakSanitizer受ptrace限制，因此命令显式关闭泄漏检测；AddressSanitizer与UndefinedBehaviorSanitizer仍启用。支持泄漏检测的本地环境可移除该环境变量。断言通过说明覆盖输入上的本地结果正确，不能替代证明、完整平台约束核对或在线评测。

## 面试追问

- 要求最坏线性时如何用 median-of-medians 选 pivot？
- 若不允许修改输入，堆法的时间/空间权衡是什么？

## 练习与提示

- 练习1：给接口注入随机引擎，让测试能复现同一分区路径。提示：随机源与选择逻辑分离。
- 练习2：实现大小k小顶堆并比较大量重复数据。提示：复杂度应写 O(n log k)，不能标成 O(n)。

## 实现复用与边界检查

大小k的小顶堆通常为 O(n log k)，大顶堆选择为 O(n+k log n)，不能据此宣称线性。随机选择应分别说明期望 O(n) 与最坏 O(n²)。
