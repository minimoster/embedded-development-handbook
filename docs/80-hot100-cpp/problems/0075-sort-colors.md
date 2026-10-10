# 75. 颜色分类：C++17 推导与实现

> 分类：技巧 · 难度：MEDIUM · 官方 Hot 100 顺序：98/100

## 题意与调用约定

数组元素只可能是0、1、2，原地按这三种值升序分类；不调用库排序函数。空数组自然不做任何操作。

题面与最新约束见 [力扣中文官方题目](https://leetcode.cn/problems/sort-colors/) / [LeetCode 官方英文题目](https://leetcode.com/problems/sort-colors/)。下文给出独立编写的推导、示例与实现；测试是本地验证，没有声称在线评测已通过。

## 从直觉到优化

两遍计数再重写可做 O(n)，但一遍扫描也能完成。荷兰国旗分区把数组切成已经确定的0区、1区、未知区、2区。每次处理未知区最左一项，把0放左、2放右，1留中。

## 不变量与正确性

保持 [0,low) 全0，[low,scan) 全1，[scan,high) 未知，[high,n) 全2。遇0与low交换，若low<scan换回的是已知1，因此scan可前进；遇2与--high交换，换回仍未知，scan不能前进；遇1直接扩大中区。未知长度每轮减少1。

## 算法步骤

1. low=scan=0，high=n，使用半开右边界。
2. 0：交换到low，同时推进low和scan。
3. 1：只推进scan。
4. 2：先缩high再交换，不推进scan。
5. 未知区为空时结束。

## 手工追踪

[2,1,0,2,0,1]：首个2与最后1交换，scan仍0得到[1,1,0,2,0,2]；扫描两个1后遇0换到low，得到[0,1,1,2,0,2]；2与右侧0交换，重检换入0，再移到左边，最终[0,0,1,1,2,2]。

## 完整 C++17 实现

以下内容来自本课 [可编译头文件](../../../algorithms/cpp/solutions/p0075_sort_colors.hpp)，namespace用于让100题可同时进入同一工程。在线提交时一般只保留评测要求的类声明，并按平台要求处理标准库命名；本地辅助头文件负责统一导入标准库。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0075 {
class Solution {
public:
    void sortColors(std::vector<int>& nums) {
        std::size_t low = 0, scan = 0, high = nums.size();
        while (scan < high) {
            if (nums[scan] == 0) std::swap(nums[low++], nums[scan++]);
            else if (nums[scan] == 1) ++scan;
            else std::swap(nums[scan], nums[--high]);
        }
    }
};
} // namespace hot100::p0075
```

## 复杂度核算

时间 O(n)，未知区每轮缩小1；额外空间 O(1)，原地修改输入，无输出容器。

## C++ 常见陷阱

- 换入右侧未知值后若递增scan会漏分类，是最常见错误。
- high为尾后边界，--high发生前有scan<high保证，不会 size_t下溢。
- else分支把所有非0/1当2，安全依赖官方仅三色前提；生产接口需验证值域。

## 测试与运行

[完整测试源文件](../../../algorithms/cpp/tests/test_p0075.cpp)：穷举长度0到10的全部三色数组，与标准排序基线对拍；包含空数组以及右侧换入仍需重检的情况。

从仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0075.cpp -o /tmp/hot100-cpp/test_p0075
/tmp/hot100-cpp/test_p0075
# 环境支持时增加运行期检查：
g++ -std=c++17 -Wall -Wextra -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0075.cpp -o /tmp/hot100-cpp/test_p0075_san
ASAN_OPTIONS=detect_leaks=0 /tmp/hot100-cpp/test_p0075_san
```

本次环境的LeakSanitizer受ptrace限制，因此命令显式关闭泄漏检测；AddressSanitizer与UndefinedBehaviorSanitizer仍启用。支持泄漏检测的本地环境可移除该环境变量。断言通过说明覆盖输入上的本地结果正确，不能替代证明、完整平台约束核对或在线评测。

## 面试追问

- 与计数重写相比，是否保留相同颜色元素的相对顺序？
- 如果元素附有对象数据而非纯整数，怎样实现稳定的三路分区？

## 练习与提示

- 练习1：为四个区间写断言，并在随机输入每次交换后验证。提示：未知区不做值限制。
- 练习2：改写为围绕pivot的小/等/大三路划分。提示：将0/1/2判断替换为与pivot比较。
