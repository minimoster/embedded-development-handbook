# RTOS 内存策略：从空闲块选择到释放合同

学习优先级：P2，选读补充，不新增核心题编号。先读 [C07 · 分配与所有权](../10-c-cpp/c07.md)、[RT08 · 栈与堆预算](rt08.md) 和 [RT10 · 参数寿命](rt10.md)。本章只展开分配策略、碎片与多区域初始化；TLS、Newlib 和任务回收细节见 [任务诊断与运行时集成](task-diagnostics.md)。

## 直接回答

分配器需要找到一个满足大小、对齐和存储属性的连续空闲块。first fit、next fit、best fit、worst fit 决定“从哪些候选块中选哪一个”；能否合并相邻空闲块是另一项能力。总空闲字节够用，仍可能没有足够大的连续块。策略名称也不会自动建立指针所有权、ISR 安全性或硬实时上界。

实现核对固定在官方 FreeRTOS-Kernel V11.1.0，提交 `dbf70559b27d39c1fdb68dfb9a32140b6a6777a0`；移植层阅读条件为单核、GCC `portable/GCC/ARM_CM4F`、非 MPU。本文没有构建内核或工具链，也没有运行模拟器、实际 heap 或目标板。下文只运行了原创 Python 整数区间模型。[固定源码](https://github.com/FreeRTOS/FreeRTOS-Kernel/tree/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0)

## 先分清四个大小

1. **请求大小 r**：调用者希望使用的有效载荷字节；不是分配器整个块的大小。
2. **头部 h 与对齐 a**：分配器可能在载荷前存放管理信息，并把总量向对齐边界取整。一个教学规则是 `c = 向上取整(r+h, a)`；实际实现还可能整块交付而不切出过小尾块。
3. **连续空闲块**：某一个可分割的连续区间。跨越在用对象、地址空洞或不同存储属性的总和，不能当成单个候选块。
4. **池总量与实际载荷**：起始地址对齐、末端哨兵、每块头部和切分阈值都会占用空间，不能把数组长度或区域长度全部预算为应用载荷。

这里把头部单列为管理开销；对齐填充和被吸收的小尾块是已占用块内部、应用不能作为额外载荷使用的空间，属于内部浪费。外部碎片则是可用空间被分散到多个空闲块，无法满足某次连续请求。不同材料可能把管理开销也计入“内部碎片”，讨论时先声明口径。

V11.1.0 的 heap_2/4/5 会先增加对齐后的 `BlockLink_t` 头部，再向 `portBYTE_ALIGNMENT` 对齐；对加法溢出和用于分配标记的最高位有检查。它们只有在剩余量严格大于 `2 × xHeapStructSize` 时才切分，否则占用整个候选块。ARM_CM4F 的对齐宏为 8；头部的编译后大小仍要由实际 ABI、类型和配置确认。本章模型指定的 h=8 不是已测得的目标结构大小。[heap_2 大小处理与切分](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/portable/MemMang/heap_2.c#L156-L267)、[heap_4 对应路径](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/portable/MemMang/heap_4.c#L173-L309)、[heap_5 对应路径](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/portable/MemMang/heap_5.c#L208-L337)、[port 对齐](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/portable/GCC/ARM_CM4F/portmacro.h#L79-L83)

## 一个原创空闲块轨迹

以下地址都是同一虚构 368 字节 arena 内的**整数偏移**，区间含起点、不含终点；没有解引用主机或 MCU 指针。空闲块长度已包含它自身的管理空间。设 h=8、a=8；模型允许保留任何正长度尾块，故意不复现 FreeRTOS 的最小切分阈值。

初始布局：

| 偏移区间 | 状态 | 长度 |
| --- | --- | --- |
| [0,80) | 空闲 F0 | 80 |
| [80,96) | 在用 L0 | 16 |
| [96,144) | 空闲 F1 | 48 |
| [144,160) | 在用 L1 | 16 |
| [160,288) | 空闲 F2 | 128 |
| [288,304) | 在用 L2 | 16 |
| [304,368) | 空闲 F3 | 64 |

总空闲量为 320，最大空闲块为 128。一次 r=33 的请求需要 `c=48`：33 字节载荷、8 字节头部、7 字节对齐填充。下面四行各自从**同一初始布局**开始，并非依次执行四次分配；大小相同的候选以较低地址打破平局。

| 策略 | 本模型的选择规则 | 选中块与剩余尾块 |
| --- | --- | --- |
| first fit | 从低地址开始，选首个长度至少 48 的块 | F0；分配 [0,48)，余 [48,80) 共 32 |
| next fit | 从保存的扫描起点继续，到末端后绕回；这里起点为 F3 的 304 | F3；分配 [304,352)，余 [352,368) 共 16 |
| best fit | 选能容纳请求的最小块 | F1；恰好 48，无尾块 |
| worst fit | 选能容纳请求的最大块 | F2；分配 [160,208)，余 [208,288) 共 80 |

next fit 在本例的后续扫描起点是剩余块 352；具体分配器如何保存、更新和回绕游标必须另行定义。“first”也依赖空闲表的顺序：地址序与大小序不是同一件事。

四个分支的剩余总量都是 272，但 worst-fit 分支的最大块已变为 80；其余分支仍保留 128。继续申请 r=113，即 c=128，前三个分支可找到块，worst-fit 分支失败。这只说明本轨迹的结果，不能据此宣布某种策略普遍最好。生命周期、请求分布、切分阈值和遍历结构变了，结果也会变。

再回到初始布局：直接申请 r=129，即 c=144，四种策略都失败，尽管总空闲量为 320。原因是没有单个 144 字节候选块。

### 合并空闲块，不等于移动在用对象

仍从初始布局出发，合法释放 L1 的整个 [144,160) 后，总空闲量增加到 336。若只把这个 16 字节块放回表中而不合并，最大块仍是 128。若支持相邻合并（coalescing），F1、刚释放的 L1 和 F2 可组成 [96,288) 共 192，于是 r=169、c=184 的请求可以满足。L0、L2 的位置没有改变。

L1 未释放时，不能把两侧空闲块跨过它合并。压缩搬移（compaction）则是另一种机制：改变在用对象的位置以集中空闲空间。普通 C/C++ 程序可能持有原始指针、内部指针、共享别名或 DMA 地址，一个抽象“搬移器”不能自由移动对象而仍假定它们全部有效；需要专门的句柄、重定位、对象可移动性和暂停使用者协议。这里核对的 heap_4/5 只合并相邻空闲块，没有搬移在用对象的压缩步骤。[heap_4 插入与合并](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/portable/MemMang/heap_4.c#L492-L559)、[heap_5 插入与合并](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/portable/MemMang/heap_5.c#L466-L533)

### buddy 是另一种分割几何

教学上的二进制 buddy 策略从满足要求的、按 2 的幂组织的块开始，递归二等分；释放时只把同阶且确实互为伙伴的两个空闲块合并，不是看到任意相邻块就合并。若本例 33 字节请求加 8 字节头部后需容纳 41，按二进制块级向上取整会需要 64：头部 8，加上本规则的块级内部浪费 23。它不能直接套在上面的任意空闲块表上，arena 对齐、分割历史和伙伴关系都必须建立。

固定块池也不等同 buddy：固定池归还的是预先划定的同类块，不需要按请求不断二分。V11.1.0 的这五个示例 heap 都没有实现本文所说的 next fit、worst fit 或 buddy。不要仅凭策略名推导 O(1) 或硬实时保证。

## V11.1.0 的 heap_1–heap_5 对照

选择并链接一份与项目配置相符的实现；不要把五份同名入口同时链接。五份源码都拒绝 `configSUPPORT_DYNAMIC_ALLOCATION=0` 的使用。表中的策略描述来自固定实现，不表示项目实际已选择它。

| 实现 | 选块与归还合同 | 容量与初始化边界 |
| --- | --- | --- |
| heap_1 | 顺序推进的 allocation-only bump；没有可用的非空块归还能力，`vPortFree` 对非空实参触发配置断言，不回收空间 | `ucHeap` / `configTOTAL_HEAP_SIZE`；分配按 port 对齐；启动期只分配仍可能耗尽 |
| heap_2 | 空闲表按大小排序，选最小足够块，即 best fit；允许释放，不合并相邻空闲块 | 同类数组堆；第一次分配初始化；释放块大小与寿命分布影响碎片 |
| heap_3 | 挂起调度器后调用 libc `malloc/free`，再恢复；具体选块、碎片和管理开销属于外部分配器 | 没有上述 `ucHeap`；`configTOTAL_HEAP_SIZE` 不配置 libc 堆；链接器、库与系统桩须配置实际可用堆 |
| heap_4 | 空闲表按地址排序，first fit；释放时合并邻接空闲块 | 单数组堆；第一次分配初始化；仍可能有被在用块隔开的外部碎片 |
| heap_5 | 多区域地址序 first fit，释放时合并邻接空闲块；单次分配仍来自一个连续候选块 | 应用先调用 `vPortDefineHeapRegions`；不能把离散区域拼成一个连续载荷 |

依据：[heap_1 分配与归还](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/portable/MemMang/heap_1.c#L47-L139)、[heap_2 排序与选择](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/portable/MemMang/heap_2.c#L96-L267)、[heap_3 包装](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/portable/MemMang/heap_3.c#L30-L94)、[heap_4](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/portable/MemMang/heap_4.c#L96-L309)、[heap_5](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/portable/MemMang/heap_5.c#L208-L337)。

heap_3 的任务间保护有前提：单核、相关使用者都遵守共同保护路径，且 libc 调用本身不需要在调度器挂起时执行阻塞操作。ISR、直接调用 libc 的其他路径、其他核心以及库内部锁不能靠这一个包装自动覆盖。源码只能证明包装调用了谁，不能证明所链接 libc 的分配策略、锁、重入性或最坏耗时；Newlib 集成检查见 [任务诊断](task-diagnostics.md)。

heap_1/2/4 的 `configAPPLICATION_ALLOCATED_HEAP=1` 只使数组由应用提供；它不会自动给任意内存地址建立容量、访问权限或链接器排他占用。`configENABLE_HEAP_PROTECTOR` 在 heap_4/5 中是可选的内部指针保护，`configHEAP_CLEAR_MEMORY_ON_FREE` 在 heap_2/4/5 中可增加清零成本。断言与这些检查都不替代合法指针合同，也不保证识别所有越界、悬空访问或 double free。[数组来源与保护分支](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/portable/MemMang/heap_4.c#L54-L138)、[配置默认值](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/include/FreeRTOS.h#L2770-L2776)、[heap_2 释放](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/portable/MemMang/heap_2.c#L289-L327)、[heap_4 释放](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/portable/MemMang/heap_4.c#L348-L395)、[heap_5 释放](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/portable/MemMang/heap_5.c#L376-L423)

## heap_5：区域描述是初始化合同

在这个固定版本中，`HeapRegion_t` 包含区域起始地址和字节数；描述表按起始地址从低到高排列，最后使用 `{NULL, 0}`。源码循环的终止条件实际是大小为零；因此不能只写一个大小非零的空地址，认为它就是终止项。[结构与 API 合同](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/include/portable.h#L138-L171)、[初始化实现](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/portable/MemMang/heap_5.c#L534-L650)

使用前逐项证明：

- **不重叠**：各区域及其完整字节范围互不重叠，不覆盖栈、全局对象、设备保留区或另一个分配器。仅检查起点有序不等于完整范围已验证；源码的顺序断言也不是全面的内存地图验证器。
- **先初始化**：在首次 `pvPortMalloc` 之前调用，包含动态创建任务、队列等引出的间接分配；正常初始化只能调用一次。不能在仍有活块时重新定义或重置堆，企图取回它们占用的空间。
- **足够大且算术合法**：起点对齐会损失前缀，每个区域还放置末端标记。检查起点加长度不会在目标地址类型中溢出，扣除前缀/标记不会下溢，有足够空间容纳有效块。
- **真实可用且长寿命**：描述表至少活到初始化调用结束；实现读取它并在区域内建立管理结构，没有长期保存描述表指针。被描述的 RAM 必须在分配器和全部活块使用期间始终存在、可访问、可写，不得来自即将退出的函数局部数组，也不得随后交给其他用途。
- **属性合适**：统一的 `pvPortMalloc(size)` 不接收“必须 DMA 可达/必须某 RAM bank”这样的区域筛选参数。地址序 first fit 不会自动替应用选择所需属性；特殊存储需求应有独立、已证明的分配合同。

例如两个各有 96 字节可用块的分离区域，总量是 192，仍不能承诺一次 128 字节连续分配成功。区域间的地址空洞和每区末端标记不会变成可拼接的载荷。

`heap_5` 的实现使用 port 的地址类型、地址排序和字节地址运算来管理目标存储地图。其正确使用还依赖实际编译器、ABI、链接器及 MCU 内存地图；不要把不同独立 C 数组之间的关系比较、减法或跨对象移动写成可移植 C 的通用规则。下文主机模型只对整数偏移做运算，不能证明这些目标层条件。[地址类型默认值](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/include/FreeRTOS.h#L598-L600)、[区域地址处理](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/portable/MemMang/heap_5.c#L560-L624)

## 上下文、所有权与失败路径

这些入口没有配套的 `pvPortMallocFromISR` / `vPortFreeFromISR` 合同；本章只讨论受控启动初始化和普通任务使用，不提供在 ISR 中调用它们的方案。ISR 若需要缓冲，应先建立独立验证的预分配/归还协议，并按 [RT06](rt06.md) 核对通信 API，而不能因任务分配器内部有保护就推断 ISR 安全。[实际公开入口](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/include/portable.h#L181-L206)

五种实现的主要分配路径都使用调度器挂起/恢复；挂起调度器不等于屏蔽所有中断。`traceMALLOC` 在挂起区内展开，失败 hook 则在分配器内部调用恢复之后执行；两者不是同一上下文。如果调用者外部仍有一层调度器挂起，内部恢复也不保证已回到正常调度。文档明确禁止在调度器挂起时调用可能导致切换的 API；挂起段内的 trace 不能安排阻塞队列或互斥量等待。失败 hook、libc 锁以及外部临界区/特殊回调的合同仍要独立审查，不能仅凭“它在任务里”获得无限制调用许可。[实际 trace 与失败 hook 顺序](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/portable/MemMang/heap_4.c#L326-L343)、[调度器挂起合同](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/include/task.h#L1576-L1683)

所有权至少写明：分配接口、合法原始地址、当前拥有者、借用者退出条件、最终释放接口和失败回滚。释放只改变分配器的块状态；不会撤销其他别名，也不会等 DMA 或别的任务停止。把一个变量设为 `NULL` 只清除那一个别名。泄漏、缓冲越界、UAF 和 double free 是不同错误，不能用“分配次数减释放次数为零”一项计数排除它们。匹配接口、在最后一次使用之后归还，完整失败回滚，详见 [C07](../10-c-cpp/c07.md) 和 [RT10](rt10.md)。

失败时检查返回值，不能先发布半初始化对象。申请数量乘元素大小需由调用方先检查溢出；分配器对其输入大小的检查不能修复在调用前已经绕回的小数。失败 hook 的配置、执行位置和工作量应单独核对，避免在 hook 中再次分配制造递归失败。

## 测量什么，才能讨论实时性

heap_2/4/5 会遍历空闲链表，成本随具体块表变化；合并需要先找到插入位置。heap_1 不扫描空闲表，但完整调用仍含对齐、调度恢复、trace/失败 hook 等路径；不能把局部 bump 操作直接当成所有配置下的固定总耗时。heap_3 的主要成本属于所链接的 libc。没有在本文建立任何硬实时上界。

对 heap_4/5，`vPortGetHeapStats` 提供总空闲量和最大空闲块等观察；这些块大小包含分配器口径，最大块值不是可直接承诺的最大应用载荷。采样后其他任务可以改变堆；统计不为下一次请求保留资源。记录请求/实际占用、最大连续块、失败原因、峰值和对象寿命分布，比只报总空闲量更有帮助。[统计字段](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/include/portable.h#L145-L156)、[heap_4 统计扫描](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/portable/MemMang/heap_4.c#L565-L599)

## 原创主机模型与验收

原创 Python 模型的 10 项单元测试全部通过，只验证整数区间选择、对齐算术、相邻合并和输入检查：

- 四种选块策略在上表中的地址、尾块、剩余总量和下一次请求结果
- 总量足够而最大块不足的失败；释放 L1 前后，合并与不合并的差别
- 小尾块被整块吸收时，占用量不同于对齐后的请求量
- 限定位宽下的加法/取整失败，以及 buddy 块级取整和伙伴关系
- 区域描述的排序、重叠、终止项、过小区域和地址溢出拒绝

这些不是 FreeRTOS heap 的测试日志；模型没有 C 指针、实际 malloc/free、任务并发、ISR、锁、硬件或 libc。模型拒绝非法区域，是模型自己的输入合同，也不表示真实 `vPortDefineHeapRegions` 会完整检查同样的错误。

后续目标实验尚未执行：固定一份 heap、完整配置和 port，保存 map/链接结果，确认数组或区域不与其他存储重叠；以有界请求轨迹覆盖成功、耗尽、碎片和失败回滚；分别测链表扫描、调度器挂起时间、允许中断的响应及 hook 成本。测试错误释放应在适当隔离的诊断环境中进行，不能在生产系统随意破坏堆。

验收时分开报告“固定源码核对”“原创主机模型”“实际内核运行”“板级时延”。一次压力测试通过，不证明所有寿命分布或所有调用上下文都安全。

## 来源与许可

本章的版本专属结论以文中链接的官方固定源码为依据；策略轨迹、区间布局和模型均为独立原创内容，没有复制实现代码。相关内核源文件使用 MIT 许可并保留各自版权声明；分发源码或其重要部分时需要保留对应声明。[V11.1.0 许可](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/LICENSE.md)

来源用于核对，不表示维护者为本章背书。SMP、MPU、其他 port、厂商分支、其他内核版本和具体 libc 都需要重新验证。
