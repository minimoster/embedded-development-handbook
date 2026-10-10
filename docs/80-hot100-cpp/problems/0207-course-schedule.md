# 207. 课程表 · C++17

> 学习位置：图论 · 第 3 课。难度：MEDIUM。这是一份独立编写的学习课，不是在线提交通过声明。

[官方题目](https://leetcode.cn/problems/course-schedule/) · 本课用简短契约与独立推演讲解，完整输入约束以官方题页为准。

## 1. 先说清输入、输出与修改边界

课程编号0..numCourses-1，先修对[a,b]表示b必须在a之前。判断能否完成所有课程，也就是依赖有向图是否无环。输入边列表只读，允许课程孤立、重复边、自环。

## 2. 从直觉到可用方案

尝试枚举课程排列代价阶乘。逐步选择“没有尚未完成先修课”的课程，能直接构造一个合法顺序：用入度记录每门课还剩多少先修边，先把入度0课程入队。

这就是Kahn拓扑排序。完成课程b后，删除b发出的依赖边，相关课程入度减一；新变0者可以开始。若所有课程都被取出，则可完成；若提前卡住，未处理部分必然含环。

## 3. 为什么正确：抓住不变量

入度始终等于来自尚未处理课程的边数，所以每个出队的0入度课程都满足所有先修条件，不会违约。若最后有未处理课程且没有0入度节点，从任何未处理节点不断追溯未处理前驱，有限图中必重复某节点，形成有向环。反过来，环中节点在环边删除前都至少有1入度，无法成为最先被取出的环节点。故处理数等于课程数当且仅当图无环。

## 4. 动手过程

1. 建邻接表out[b].push_back(a)，增加indegree[a]。
2. 所有入度0课程入队，包括没有出现在边表中的孤立课程。
3. 出队一门课，计数completed++。
4. 遍历后继，入度减一，恰好减到0时入队。
5. completed==numCourses返回true，否则false。

## 5. 原创例子逐步追踪

5门课，依赖[2,0]、[2,1]、[3,2]，课程4孤立。初始入度[0,0,2,1,0]，队列0,1,4。完成0后2仍有1入度；完成1后2入队；完成4不影响其他课；接着2解锁3，总共完成5门。再加[1,3]会留下1→2→3→1环，队列耗尽后计数不足5。

## 6. 时间、栈与输出空间分别算

设V为课程数，E为先修对数。建立图和每条边的入度更新均O(V+E)，辅助空间邻接表O(V+E)、入度与队列O(V)，总O(V+E)。无递归栈，返回bool的输出空间O(1)。重复边计作多重边，增加与减少次数配对，不改变正确性。

## 7. C++17实现与所有权

- 方向要根据[a,b]写成b→a；反向图虽同样保留环，但无法直接输出题意上的选课顺序。
- 图与计数全部为局部变量，同一Solution可反复调用。
- 自环令课程入度至少1，必然无法全部处理。
- 入度使用int基于官方E范围；极大图需更宽类型。
- 不修改prerequisites，拓扑操作作用于局部入度数组。

下面是本仓库完整的头文件实现；support.hpp只提供标准库包含与题目节点声明，算法自身不依赖测试工具。在线评测通常已提供节点类型，提交时保留对应类并使用评测环境要求的作用域。

```cpp
#pragma once
#include "../support.hpp"

namespace hot100::p0207 {
class Solution {
public:
    bool canFinish(int numCourses, std::vector<std::vector<int>>& prerequisites) {
        std::vector<std::vector<int>> outgoing(numCourses);
        std::vector<int> indegree(numCourses, 0);
        for (const auto& dependency : prerequisites) {
            int after = dependency[0], before = dependency[1];
            outgoing[before].push_back(after);
            ++indegree[after];
        }
        std::queue<int> ready;
        for (int course = 0; course < numCourses; ++course)
            if (indegree[course] == 0) ready.push(course);
        int completed = 0;
        while (!ready.empty()) {
            int course = ready.front();
            ready.pop();
            ++completed;
            for (int next : outgoing[course]) if (--indegree[next] == 0) ready.push(next);
        }
        return completed == numCourses;
    }
};
} // namespace hot100::p0207
```

[查看可编译源文件](../../../algorithms/cpp/solutions/p0207_course_schedule.hpp) · [查看完整测试](../../../algorithms/cpp/tests/test_p0207.cpp)

## 实现复用与边界检查

反向建边不改变是否存在有向环。三色 DFS 或 Kahn 方法都需重新初始化邻接、访问和结果状态；此实现采用局部结构并测试连续调用。

## 9. 本地验证

450个随机有向图与Warshall传递闭包的对角判环参照比较；同对象复用、自环、重复边、孤立课和零课程扩展。 测试采用固定种子，可复现每轮输入。断言针对返回值，也针对应保持的地址、连接或输入状态。内存由arena或对应destroy工具明确回收。本地测试通过仅说明这些可复现用例通过，不等于完整在线评测结论。

从仓库根目录运行：

```bash
mkdir -p /tmp/hot100-cpp
c++ -std=c++17 -Wall -Wextra -pedantic algorithms/cpp/tests/test_p0207.cpp -o /tmp/hot100-cpp/test_p0207
/tmp/hot100-cpp/test_p0207
# 编译器支持时，再检查地址、未定义行为与泄漏
c++ -std=c++17 -Wall -Wextra -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/tests/test_p0207.cpp -o /tmp/hot100-cpp/test_p0207_san
ASAN_OPTIONS=detect_leaks=1 /tmp/hot100-cpp/test_p0207_san
# 若执行环境在ptrace下明确拒绝LeakSanitizer，可仅运行地址/UB检查：
# ASAN_OPTIONS=detect_leaks=0 /tmp/hot100-cpp/test_p0207_san
```

## 10. 面试延伸

- 要输出一种可行课序，把哪些内容保存下来？按出队顺序追加编号。
- 要找字典序最小拓扑顺序，队列要换成什么？小根堆，复杂度增加到O((V+E)logV)上界。
- “可以完成”是否意味着顺序唯一？若某一步有多个可选0入度节点，通常存在多个拓扑序。

## 11. 两道练习与提示

1. 写三色DFS参照，明确0/1/2分别表示未访问、当前路径、已完成。提示：只有指向1色节点的边是回边。
2. 输出一个真实环供诊断。提示：DFS保存父节点，在遇到1色目标时逆着父链还原闭环。
