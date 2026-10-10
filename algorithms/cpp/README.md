# Hot100 C++17本地工程

本目录是[中文完整学习轨道](../../docs/80-hot100-cpp/README.md)的可编译源码与测试。

- solutions：当前官方100题，独立头文件、独立namespace
- supplementary：额外公开练习695，单列，不混入100
- tests：每题一个main，使用assert、自建数据、固定种子或小规模穷举
- support.hpp：标准头、目标表示的编译期断言、ListNode/TreeNode/Node、本地测试arena及输出销毁工具
- exercises：LC53分治与LC295两类官方追问的独立可执行练习，不增加题单数量
- validate.py：库存、源码/文档一致性、链接、逐题编译运行、全部头文件同包编译

```sh
python3 algorithms/cpp/validate.py --sanitizers --debug-iterators
```

需从仓库根运行，Python3与g++支持C++17。不要加`-DNDEBUG`，否则assert会被关闭。ASan/UBSan与libstdc++ checked iterators是不同检查，任何一种都不能证明全部输入正确。LeakSanitizer由于执行环境限制未运行。

本地目标的 `int` 值域固定为 [-2147483648,2147483647]，`long long` 至少63个值位，`char` 为8位，英文字母和数字使用ASCII编码。C++17不保证这些目标属性；support.hpp会检查它们。256项计数表只建模字节，不是Unicode字符表。断言通过不等于某款嵌入式工具链已验证，详情见[表示契约](../../docs/80-hot100-cpp/SOURCES.md)。

追问练习单独运行，不改动正式100题和LC695的入口：

```sh
g++ -std=c++17 -O2 -Wall -Wextra -Werror -pedantic algorithms/cpp/exercises/test_followup_variants.cpp -o /tmp/hot100-followups
/tmp/hot100-followups
```

在线平台通常已经定义节点，只保留题目所需类和标准头即可；namespace与arena是本地包装。正式题号、文件、复杂度与输入修改约定均在对应课文。未执行在线判题提交。
