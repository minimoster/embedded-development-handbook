# 739. 每日温度：C++17 推导与实现

> 分类：栈 · 难度：MEDIUM · 官方 Hot 100 顺序：72/100

## 题意与调用约定

每一天返回距离下一次严格更高温度需要等待的天数；后面没有更高温度返回0。相同温度不能结束等待。

题面与最新约束见 [力扣中文官方题目](https://leetcode.cn/problems/daily-temperatures/) / [LeetCode 官方英文题目](https://leetcode.com/problems/daily-temperatures/)。下文给出独立编写的推导、示例与实现；测试是本地验证，没有声称在线评测已通过。

## 从直觉到优化

每个位置向右找第一项更大值要 O(n²)。把尚未找到答案的日子压栈，新一天到来时，它可以一次解决若干更冷的旧日子。剩余栈按温度非递增；栈顶最容易被当前温度解决。

## 不变量与正确性

pending 下标递增、对应温度非递增，且所有栈中下标都未遇到更暖天。当前温度大于栈顶时，当前就是该栈顶的第一个更暖天：若以前有更暖天，该项早已被弹出。相等温度不弹，保证“严格更高”。

## 算法步骤

1. 答案预置0，表示扫描结束仍未解决的项。
2. 新日 i 到来，反复弹出温度严格低于它的栈顶 j。
3. 对每个被弹位置填写 i-j。
4. 最后把 i 压栈等待未来处理。

## 手工追踪

[60,60,58,64,61,70]。前三天栈[0,1,2]；第3天64依次解决2、1、0，写1、2、3。第4天61等待；第5天70解决4和3，写1与2。第5天本身没有未来更暖日，保持0。

## 完整 C++17 实现

以下内容来自本课 [可编译头文件](../../../algorithms/cpp/solutions/p0739_daily_temperatures.hpp)，namespace用于让100题可同时进入同一工程。在线提交时一般只保留评测要求的类声明，并按平台要求处理标准库命名；本地辅助头文件负责统一导入标准库。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0739 {
class Solution {
public:
    std::vector<int> dailyTemperatures(std::vector<int>& temperatures) {
        int n = static_cast<int>(temperatures.size());
        std::vector<int> answer(n, 0), pending;
        for (int i = 0; i < n; ++i) {
            while (!pending.empty() && temperatures[pending.back()] < temperatures[i]) {
                int j = pending.back(); pending.pop_back();
                answer[j] = i - j;
            }
            pending.push_back(i);
        }
        return answer;
    }
};
} // namespace hot100::p0739
```

## 复杂度核算

时间 O(n)，因为每个下标至多入栈与出栈各一次，while 的总次数为 O(n)。辅助栈空间 O(n)，输出 O(n)；无递归。

## C++ 常见陷阱

- 比较必须是 <，若写 <= 会把相同温度误当更暖。
- 栈存下标而不是只存温度，才能计算距离并定位输出。
- vector 的 back 引用在 pop 后失效；先把下标复制到局部变量再弹。

## 测试与运行

[完整测试源文件](../../../algorithms/cpp/tests/test_p0739.cpp)：1500组固定种子温度数组与逐日向右搜索的二重循环对拍；专测相等温度、递减数组与空数组。

从仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0739.cpp -o /tmp/hot100-cpp/test_p0739
/tmp/hot100-cpp/test_p0739
# 环境支持时增加运行期检查：
g++ -std=c++17 -Wall -Wextra -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0739.cpp -o /tmp/hot100-cpp/test_p0739_san
ASAN_OPTIONS=detect_leaks=0 /tmp/hot100-cpp/test_p0739_san
```

本次环境的LeakSanitizer受ptrace限制，因此命令显式关闭泄漏检测；AddressSanitizer与UndefinedBehaviorSanitizer仍启用。支持泄漏检测的本地环境可移除该环境变量。断言通过说明覆盖输入上的本地结果正确，不能替代证明、完整平台约束核对或在线评测。

## 面试追问

- 从右向左扫描时，栈里要丢弃哪些“没有资格成为下一更大值”的项？
- 若等待规则改为至少高出k度，普通单调栈是否仍适用？

## 练习与提示

- 练习1：改为寻找下一次不低于当天的温度。提示：弹出条件改成 <=。
- 练习2：实现右向左版本并对拍。提示：丢弃温度不大于当前项的栈顶，再取最近候选。
