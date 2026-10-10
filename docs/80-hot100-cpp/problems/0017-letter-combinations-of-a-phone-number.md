# 17. 电话号码的字母组合（C++17）

> 回溯 · MEDIUM。前置：能读懂本题使用的容器与递归/循环状态。建议先手推，再自己写，最后看实现。

## 题意与接口边界

每个数字2..9映射到电话按键上的一组字母，生成逐位选择一个字母的所有字符串。空输入返回空列表，本地对非法数字也返回无组合。

题目与完整输入约束：[官方题页](https://leetcode.cn/problems/letter-combinations-of-a-phone-number/)。本页为原创教学表达，不重复完整官方题面。

## 从朴素思路走向主解

结果是各数字对应字符集合的笛卡尔积。每一层只负责一个数字，分支数量取决于该键有3个还是4个字母；无需used，因为不同数字位置可以选择相同字母。

## 状态与执行步骤

建立固定映射；visit(index)代表已经完成前index位。遍历当前数字的所有字母，追加一个字符，递归下一位并回退；完成全部位时复制path输出。

## 手推一组自建例子

“27”第一位a/b/c，第二位p/q/r/s，共3×4=12项。a分支为ap、aq、ar、as；再b分支；最后c分支止于cs。重复调用“2”应只返回a,b,c，不含先前结果。

读到这里先暂停：不看代码，画出每次选择/指针移动后的状态；必须能解释为什么这一步安全，才算理解。

## 为什么正确

入口path长度等于index，每个已有字符来自对应数字映射。下一层穷尽该位置全部合法字母；完整路径每位合法。任意结果串有唯一逐位选择路线，因此无遗漏无重复。

## 完整 C++17 实现

源码：[可编译头文件](../../../algorithms/cpp/solutions/p0017_letter_combinations_of_a_phone_number.hpp)。本地包装的namespace用于把100题同时编译；在线平台提交时只保留类及所需标准头，链表/树节点以平台提供定义为准。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0017 {
class Solution {
public:
    std::vector<std::string> letterCombinations(std::string digits) {
        if (digits.empty()) return {};
        const std::array<std::string, 10> letters{"", "", "abc", "def", "ghi", "jkl", "mno", "pqrs", "tuv", "wxyz"};
        std::vector<std::string> answer;
        std::string path;
        std::function<void(std::size_t)> visit = [&](std::size_t index) {
            if (index == digits.size()) { answer.push_back(path); return; }
            const char digit = digits[index];
            if (digit < '2' || digit > '9') return;
            for (char c : letters[digit - '0']) {
                path.push_back(c); visit(index + 1); path.pop_back();
            }
        };
        visit(0); return answer;
    }
};
} // namespace hot100::p0017
```

## 复杂度与存储边界

若长度n且有a个3字母键、b个4字母键，结果数Q=3^a·4^b；时间O(nQ)，输出O(nQ)，辅助path/栈O(n)。常写O(n4^n)是上界而非所有输入的精确数量。

## C++与边界陷阱

数字字符转索引需减'0'，不能直接以ASCII码索引10槽数组。0/1没有本题映射，泛化时需定语义。若实现使用成员res，同一实例多次调用前要清理；此实现全部局部状态避免残留。

## 面试追问：先说变化，再改算法

迭代法逐位扩展已有字符串也可，但会形成每层中间结果。若输入很长，可懒生成或回调输出，用户需接受指数数量带来的时间下界。

## 自测与迁移练习

练习1：实现迭代笛卡尔积并比较峰值内存，提示每层新旧结果可能同时存在。练习2：增加自定义键盘映射，提示映射为空的数字是无解还是跳过，必须先写契约。

本题[可运行测试](../../../algorithms/cpp/tests/test_p0017.cpp)包含普通、边界或重复调用用例；有适合独立oracle的题还做固定种子的随机/穷举对拍。测试不是在线判题AC记录。仓库根目录可单独运行：

```sh
g++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0017.cpp -o /tmp/hot100-p0017
/tmp/hot100-p0017
```

复习门槛：能用自己的话复述状态含义，手推一组未见输入，指出复杂度包含哪些存储，再闭卷实现并通过测试。只记模板而说不出不变式，还需要回到手推。
