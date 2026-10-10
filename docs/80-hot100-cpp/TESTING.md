# 可复现验证与诚实边界

## 一条命令运行

从仓库根目录：

```sh
python3 algorithms/cpp/validate.py --sanitizers --debug-iterators
```

需Python3和支持C++17的g++。脚本默认4个编译任务并行；资源紧张可加`--jobs 1`。只跑某题可加`--ids 1,76,146`；部分运行仍报告全仓库存完整性，不能用部分结果冒充全100覆盖。机器报告默认写到本目录的`validation-result.json`。

脚本检查100个正式题号与1个单列补充题、17分类数量、每题文档/源码/测试存在性、完整源码与文档代码块一致、相对链接、逐题独立编译与运行，并将所有头文件同时包含做语法检查。所有编译开启`-std=c++17 -Wall -Wextra -Werror -pedantic`，普通模式`-O2`；地址/未定义行为模式`-O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer`；checked-iterator模式另启用`-D_GLIBCXX_DEBUG -D_GLIBCXX_ASSERTIONS`，能发现仅构造非法迭代器而未读写内存的错误。

## 测试具体在检查什么

- 每个入口均有普通与边界用例；有状态的类或搜索还检查重复调用、缓存淘汰、对象独立或输入恢复
- 许多数值/数组题以O(n²)/O(n³)枚举、排序、位掩码、简单DP等独立参考算法固定种子随机对拍
- 链表看节点地址、连边顺序、成环/无环与输出所有权；树看遍历/结构/路径，不能只比值集合
- 回溯核对数量、唯一性、合法性以及适用的小规模枚举；指数题规模故意受限
- ASan查实际运行路径上的越界/use-after-free等，UBSan查实际路径上的有符号溢出等未定义行为

附加的[独立小域穷举](../../algorithms/cpp/tests/independent_exhaustive.cpp)另覆盖17个高风险题号，使用不同枚举/参考算法，并启用checked iterators；命令加`--exhaustive`可重跑。它不增加题数，不把小域穷举冒充全部输入证明。

具体用例与种子直接位于各题测试文件；入口数不是断言数，也不是随机输入总数。没有以测试文件行数虚报覆盖率。

## 官方追问的独立回归

[LC53分治/LC295范围变体源码](../../algorithms/cpp/exercises/followup_variants.hpp)与[单独执行入口](../../algorithms/cpp/exercises/test_followup_variants.cpp)不属于题单新增入口，库存仍为100正式+1补充。库存验证器不自动运行这份练习，须另执行：

```sh
g++ -std=c++17 -O2 -Wall -Wextra -Werror -pedantic algorithms/cpp/exercises/test_followup_variants.cpp -o /tmp/hot100-followups
/tmp/hot100-followups
g++ -std=c++17 -O1 -g -Wall -Wextra -Werror -pedantic -D_GLIBCXX_DEBUG -D_GLIBCXX_ASSERTIONS algorithms/cpp/exercises/test_followup_variants.cpp -o /tmp/hot100-followups-debug
/tmp/hot100-followups-debug
g++ -std=c++17 -O1 -g -Wall -Wextra -Werror -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer algorithms/cpp/exercises/test_followup_variants.cpp -o /tmp/hot100-followups-san
ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1 /tmp/hot100-followups-san
```

LC53用独立区间枚举核对四项摘要，穷举三值字母表长度1到8的9840个非空数组，检查任意非空切分的合并，以及100000项正/负数边界。LC295穷举七值字母表长度1到5的19607条非空流，每个前缀与排序基线比较；另有2500项带内固定种子流、50000次带内插入、99%秩偏移与早期带外中位数反例。计数版在所需秩落到带外时必须明确拒绝，不能返回截断后的数值；精确版保留有序离群值。执行入口还核对LC72交换轴后答案、LC76再次右扩的两个前缀、LC198前缀最优不要求选末房的例子。

这份回归仍只验证有限输入与执行路径；复杂度、比例承诺的时机和表示契约另由课文说明。所有入口均包含support.hpp的编译期表示检查，通过不代表已在其他工具链或硬件运行。

## 内存生命周期

support.hpp的arena用unique_ptr拥有测试输入节点。算法接到的是借用指针，不能delete这些输入。新构造的链表/树输出由对应destroy帮助函数回收；LRU使用list和hash，Trie使用vector保存节点记录、以索引连接，容器自动管理存储生命周期。输入节点被unlink不代表它自动释放，实际项目应按拥有者约定回收。

执行器在ptrace环境下无法运行LeakSanitizer；ASan设置`detect_leaks=0`。因此“地址/UB检查通过”不等于“泄漏扫描通过”或“证明没有泄漏”。源码所有权审查和显式清理只是补充证据，报告会保留这个限制。

## 这些结论不能从报告推出来

本地测试通过不是在线判题AC；固定种子有限对拍不是所有输入穷举；sanitizer不覆盖没执行的路径；官方输入内正确不等于支持任意畸形矩阵/负候选/超大数值；C++哈希均摊性能也不等于硬实时延迟保证。逐题正确性解释、输入前提和复杂度说明必须一起读。

当前快照采用schema_version=2，以snapshot_id标识这轮知识修订，并记录公开base_commit；验证器检查完整文件集合与逐文件SHA-256，不能混用不同快照。最终文件SHA-256清单见`freeze-manifest.json`，用于发布前核对内容是否又被修改。重跑验证时报告时间与执行记录会变化，不能把旧freeze清单当成新版本。
