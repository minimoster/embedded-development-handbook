# 4. 寻找两个正序数组的中位数：C++17 推导与实现

> 分类：二分查找 · 难度：HARD · 官方 Hot 100 顺序：68/100

## 题意与调用约定

两个数组各自非递减，合并后至少有一个元素，返回合并序列的中位数。偶数长度取中间两值的平均；算法不需要真的合并。

题面与最新约束见 [力扣中文官方题目](https://leetcode.cn/problems/median-of-two-sorted-arrays/) / [LeetCode 官方英文题目](https://leetcode.com/problems/median-of-two-sorted-arrays/)。下文给出独立编写的推导、示例与实现；测试是本地验证，没有声称在线评测已通过。

## 从直觉到优化

归并 O(m+n) 能作为可靠基线，但题目要求更快。中位数只关心一个切分：左侧恰有 ceil((m+n)/2) 项，而且任何左值不大于任何右值。决定较短数组取几项后，较长数组的左侧项数随之确定，只剩一个二分变量。

## 不变量与正确性

切分 i、j 满足 i+j=leftCount。数组各自有序，因此只要 a左末<=b右首 且 b左末<=a右首，全局切分就合法。若 a左末>b右首，i过大；若 b左末>a右首，i过小。较短数组保证所有 i∈[0,m] 导出的 j∈[0,n]，边界用虚拟无穷表示。

## 算法步骤

1. 确保 a 是较短数组；交换只多一个固定深度调用。
2. 二分 a 左侧项数 i，从0到m，j=leftCount-i。
3. 用四个边界值检验交叉不等式，偏大向左、偏小向右。
4. 合法切分时，奇数取两左末的最大值；偶数再与两右首的最小值平均。

## 手工追踪

a=[1,7]，b=[2,3,9]，左侧需3项。i=1,j=2：a左1、a右7、b左3、b右9；1<=9且3<=7，切分合法。左侧 [1,2,3]，右侧[7,9]，中位数3。a空时 i=0，直接利用 b 的两个中心邻值。

## 完整 C++17 实现

以下内容来自本课 [可编译头文件](../../../algorithms/cpp/solutions/p0004_median_of_two_sorted_arrays.hpp)，namespace用于让100题可同时进入同一工程。在线提交时一般只保留评测要求的类声明，并按平台要求处理标准库命名；本地辅助头文件负责统一导入标准库。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0004 {
class Solution {
public:
    double findMedianSortedArrays(std::vector<int>& a, std::vector<int>& b) {
        if (a.size() > b.size()) return findMedianSortedArrays(b, a);
        int m = static_cast<int>(a.size()), n = static_cast<int>(b.size());
        if (m + n == 0) throw std::invalid_argument("median requires data");
        int lo = 0, hi = m, leftCount = (m + n + 1) / 2;
        while (lo <= hi) {
            int i = lo + (hi - lo) / 2, j = leftCount - i;
            long long al = i ? a[i-1] : std::numeric_limits<long long>::lowest();
            long long ar = i < m ? a[i] : std::numeric_limits<long long>::max();
            long long bl = j ? b[j-1] : std::numeric_limits<long long>::lowest();
            long long br = j < n ? b[j] : std::numeric_limits<long long>::max();
            if (al > br) hi = i - 1;
            else if (bl > ar) lo = i + 1;
            else {
                long long left = std::max(al, bl);
                if ((m + n) % 2) return static_cast<double>(left);
                return (static_cast<double>(left) + static_cast<double>(std::min(ar, br))) / 2.0;
            }
        }
        throw std::logic_error("inputs must be sorted");
    }
};
} // namespace hot100::p0004
```

## 复杂度核算

时间 O(log(min(m,n)+1))，额外空间 O(1)。为调整短数组的递归深度最多2层；没有线性归并输出。

## C++ 常见陷阱

- 先转 double 再求和，避免两个 INT_MAX 的 int 加法溢出。
- 虚拟无穷用 long long 极值，真实 int 数据不会与哨兵混淆。
- 长度转 int 与 m+n 要在 int 可表示范围；两个数组都空没有数学中位数。
- 输入未排序时不能保证一定检测出错误，抛错分支只是防御，不能替代排序前提。

## 测试与运行

[完整测试源文件](../../../algorithms/cpp/tests/test_p0004.cpp)：2000组固定种子双有序数组与合并排序基线对拍；含一侧为空、重复值、INT_MIN/INT_MAX平均值。

从仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0004.cpp -o /tmp/hot100-cpp/test_p0004
/tmp/hot100-cpp/test_p0004
# 环境支持时增加运行期检查：
g++ -std=c++17 -Wall -Wextra -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0004.cpp -o /tmp/hot100-cpp/test_p0004_san
ASAN_OPTIONS=detect_leaks=0 /tmp/hot100-cpp/test_p0004_san
```

本次环境的LeakSanitizer受ptrace限制，因此命令显式关闭泄漏检测；AddressSanitizer与UndefinedBehaviorSanitizer仍启用。支持泄漏检测的本地环境可移除该环境变量。断言通过说明覆盖输入上的本地结果正确，不能替代证明、完整平台约束核对或在线评测。

## 面试追问

- 如何推广到寻找两数组合并后的第k小元素？
- 若必须保证最坏 O(log(m+n))，为什么不能用普通 quickselect？

## 练习与提示

- 练习1：追踪 a=[4]、b=[1,2,3,5,6] 的切分。提示：最初 i=0，观察 b左与a右。
- 练习2：实现线性归并参考解并随机对拍。提示：比较空数组、重复值与极值。
