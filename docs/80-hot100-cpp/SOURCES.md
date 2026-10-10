# 官方题单与实现边界

[力扣官方Hot100学习计划](https://leetcode.cn/studyplan/top-100-liked/)按2026-10-10核对为100题、17类，题号和官方链接见[题单清单](manifest.json)。[LC695额外练习](supplementary/0695-max-area-of-island.md)单列，不计入正式100题。

课程包含独立编写的讲解、推演、实现与测试；完整题面和最新输入约束以每题的官方链接为准。算法思想、题号与第三方作品的具体表达应分别对待。外部题目与官方文档的权利归各自权利人，不把公开链接当作转载许可。

本地通过不等于在线判题AC、所有输入证明或实际部署验证。部分实现对空输入、非法k等额外边界有明确约定，其他前提仍遵循题目契约。公共节点定义和测试arena仅用于本地学习；平台已有节点类型时不能重复提交。namespace用于同仓库避免名称冲突。

[实现风险与复用提醒](ERRATA.md)区分合法输入、重复调用、生命周期、接口扩展与复杂度模型。

## 本地可运行目标的表示契约

本轨道明确选择 `int` 值域为 [-2147483648,2147483647]、`long long` 至少63个值位（另有符号位）、8位 `char`，并要求英文字母和十进制数字采用ASCII编码。字母因此分别连续，`ch-'a'` 等索引才与本课的26项表对应。C++17语言本身不保证32位 `int` 值域、8位字节或ASCII字母编码；这里是所选目标的契约，不是所有C++17实现的属性。

[support.hpp](../../algorithms/cpp/support.hpp) 用编译期断言检查值域、字节宽度和全部大小写字母/数字的编码。不满足契约的目标会拒绝编译；通过断言也只核对这些表示条件，不能据此声称已支持或验证某款MCU、MSVC、栈容量、分配器或实时延迟。移植时仍须核对官方输入范围、工具链和运行期约束，必要时改用明确宽度的接口或显式字符映射，并重新验证。

LC3、LC76的256项表按8位字节建模。先转 `unsigned char` 解决负下标，8位字节契约保证索引在0到255；两者缺一不可。这不是通用Unicode字符表；UTF-8多字节序列、码点和用户感知字符需要另行定义与实现。LC70的n=46结果超过本目标32位有符号 `int` 上界，而不是超过所有符合C++17的 `int` 上界。

## C++17容器与平台口径的一手依据

核对日期2026-10-10。以下C++草案链接固定到N4659（C++17工作草案的公开排版），用于核准本课程复杂度与迭代器规则，不把当前更新草案的新接口当作C++17功能。

- [N4659基本类型](https://timsong-cpp.github.io/cppwp/n4659/basic.fundamental)、[数值范围](https://timsong-cpp.github.io/cppwp/n4659/support.limits)与[字符集](https://timsong-cpp.github.io/cppwp/n4659/lex.charset)：整数表示与执行字符集由目标决定；标准保证数字连续，不保证英文字母连续或ASCII编码。
- [N4659 priority_queue成员](https://timsong-cpp.github.io/cppwp/n4659/priqueue.members)：push先调用底层容器push_back再调整堆；LC295因此区分均摊插入与单次扩容成本。
- [N4659 vector概述](https://timsong-cpp.github.io/cppwp/n4659/vector.overview)：末端插入按均摊常数讨论。
- [N4659 vector修改操作](https://timsong-cpp.github.io/cppwp/n4659/vector.modifiers)：重分配的引用、指针与迭代器失效规则；本课Trie因此持索引而非跨扩容引用。
- [N4659 deque修改操作](https://timsong-cpp.github.io/cppwp/n4659/deque.modifiers)：单元素两端插入的常数复杂度；本课155的主容器选择依据。
- [N4659 unordered容器要求](https://timsong-cpp.github.io/cppwp/n4659/unord.req)：平均与最坏复杂度，以及rehash的迭代器失效边界。哈希不是所有输入最坏常数保证。
- [Microsoft C++数据类型范围](https://learn.microsoft.com/en-us/cpp/cpp/data-type-ranges?view=msvc-170)：该平台long为4字节，不能以“long必64位”论证差值安全。当前仓库实际运行的是Linux g++，没有声称也已在MSVC或MCU交叉编译器运行。
