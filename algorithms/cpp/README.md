# Hot100 C++17本地工程

本目录是[中文完整学习轨道](../../docs/80-hot100-cpp/README.md)的可编译源码与测试。

- solutions：当前官方100题，独立头文件、独立namespace
- supplementary：额外公开练习695，单列，不混入100
- tests：每题一个main，使用assert、自建数据、固定种子或小规模穷举
- support.hpp：标准头、ListNode/TreeNode/Node、本地测试arena及输出销毁工具
- validate.py：库存、源码/文档一致性、链接、逐题编译运行、全部头文件同包编译

```sh
python3 algorithms/cpp/validate.py --sanitizers --debug-iterators
```

需从仓库根运行，Python3与g++支持C++17。不要加`-DNDEBUG`，否则assert会被关闭。ASan/UBSan与libstdc++ checked iterators是不同检查，任何一种都不能证明全部输入正确。LeakSanitizer由于执行环境限制未运行。

在线平台通常已经定义节点，只保留题目所需类和标准头即可；namespace与arena是本地包装。正式题号、文件、复杂度与输入修改约定均在对应课文。未执行在线判题提交。
