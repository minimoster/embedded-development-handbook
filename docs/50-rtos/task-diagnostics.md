# FreeRTOS 任务诊断与运行时集成

学习优先级：P1。本文是源码补充，不新增核心题编号。先读 [任务生命周期](kernel-task-lifecycle.md)、[RT07](rt07.md) 和 [RT08](rt08.md)，再用本章检查诊断功能是否改变了资源、时间和寿命合同。

## 直接回答

TCB 的可选字段不只是“方便调试的小变量”。打开追踪、运行时间统计、应用标签或 C 运行时支持，会改变 TCB 大小、创建和删除路径、切换开销以及应用需要承担的清理责任。诊断数据是有时效和寿命限制的观察；它不能替代同步，也不能证明一个已删除任务的句柄仍然有效。

本章重点回答：谁拥有 TLS 指向的对象，谁真正调用任务标签，运行时间计数到底量了什么，以及为什么打开 Newlib 支持后仍需要检查 libc 的锁和内存管理。

## 版本与适用范围

- 官方 FreeRTOS-Kernel V11.1.0，固定提交 `dbf70559b27d39c1fdb68dfb9a32140b6a6777a0`
- 单核，`configNUMBER_OF_CORES=1`，GCC `portable/GCC/ARM_CM4F`，非 MPU
- 32 位 tick；tick 频率由项目指定；运行时间统计源是另一项独立选择
- 每个可选功能只在对应配置打开时讨论；静态与动态分配按实际项目选择

这些是阅读条件，不是一份已编译通过的板级配置。本文完成了固定源码核对和下文的原创 Python 算术模型测试；没有构建或运行 FreeRTOS 内核、ARM 模拟器或目标板，也没有验证某一版 Newlib 工具链。[固定提交](https://github.com/FreeRTOS/FreeRTOS-Kernel/tree/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0)

## 先确定配置合同

TCB 的布局受条件编译控制。应用与内核应使用一致的配置，不要把内部字段偏移当成稳定 ABI；静态任务应使用该版本的 `StaticTask_t`，不要自己复制一个 TCB 结构。[TCB 可选字段](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L358-L436)、[静态任务结构与检查](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L1266-L1299)

| 功能 | V11.1.0 的主要开关或前提 | 集成时新增的责任 |
| --- | --- | --- |
| 追踪编号与系统快照 | `configUSE_TRACE_FACILITY=1` | 额外字段、快照成本和对象寿命；并不自动安装记录器 |
| 应用任务标签 | `configUSE_APPLICATION_TASK_TAG=1` | 标签类型、调用上下文与更换时机 |
| 应用 TLS 指针槽 | `configNUM_THREAD_LOCAL_STORAGE_POINTERS>0` | 约定索引，管理指针对象的同步、拥有权与释放 |
| 运行时间计数 | `configGENERATE_RUN_TIME_STATS=1`；配置和读取统计源宏 | 计数源、宽度、频率、回绕、读一致性与测量口径 |
| 运行时间格式化 `vTaskGetRunTimeStatistics` | 实现同时要求 trace、运行统计与 `configUSE_STATS_FORMATTING_FUNCTIONS>0`；格式化配置要求动态分配支持 | 临时数组、输出缓冲、`snprintf`、栈和执行时间 |
| Newlib 适配 | `configUSE_NEWLIB_REENTRANT=1` | 本版本转入 C-runtime TLS 接口；仍需核对工具链、libc 锁和系统桩 |
| 提前解除阻塞 | `INCLUDE_xTaskAbortDelay=1` | 额外唤醒原因和业务停止协议 |

原始 `uxTaskGetSystemState` 只要求 trace 功能；未开启运行时间统计时，相应计数为零，不能把零解释为“任务从未运行”。某个宏已定义也不等于硬件源实现正确。[配置检查](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/include/FreeRTOS.h#L2657-L2676)、[格式化配置与计数类型](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/include/FreeRTOS.h#L2818-L2845)、[格式化函数实际编译条件](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L7381-L7398)

## 追踪编号与任务标签

### 两个编号有不同用途

`uxTCBNumber` 由内核在创建路径赋值，用来辅助辨认删除后又创建的任务。`uxTaskNumber` 留给追踪工具使用，`vTaskSetTaskNumber` 和 `uxTaskGetTaskNumber` 操作的是后者。`TaskStatus_t.xTaskNumber` 填入的却是前者。名字相近，不代表 API 返回同一个编号。[创建编号](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L2067-L2077)、[追踪编号 API](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L5616-L5663)、[快照赋值](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L6126-L6166)

例如，地址 H 曾属于任务 A，A 删除后分配器可能把 H 给任务 B。日志只用 H 合并行，会把两个任务混在一起。创建编号能在有限观察窗口内帮助区分，但它是有限宽度计数，不能承诺永久唯一，更不能使旧 H 重新成为合法句柄。长期遥测可以另外记录应用分配的实例编号和启动标识，保留已复制的历史记录，停止查询旧句柄。

### 设置标签不会自动执行标签函数

本版本的标签类型是 `TaskHookFunction_t`。`vTaskSetApplicationTaskTag` 保存标签；只有显式调用 `xTaskCallApplicationTaskHook`，或应用自行把调用接到某个 trace 宏，才会执行函数。未设置标签时，调用入口返回 `pdFAIL`。[标签保存、获取与调用](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L4923-L5056)

还有一个容易误判的上下文：任务 M 调用任务 W 的 hook，函数是在 M 的调用栈上同步执行，不是向 W 投递请求，也不会自动切换到 W。若 hook 内用 `NULL` 查询“当前任务”的 TLS，查到的是当前调用者的槽，不能以为一定查到 W 的槽。需要目标上下文时，应由参数明确传递，并保护其寿命。

不要把任意对象指针强转为函数指针后再调用。若把标签用于特殊诊断编码，需要单独核对实现和 ABI，不能据此获得可移植的对象存储合同。需要对象指针时，优先使用明确的 TLS 指针槽或应用注册表。setter 的临界区只保护其保存步骤，不能替应用保证 hook 的被指向资源寿命；最好在初始化阶段安装，动态更换时有共同遵守的协议。接入切换 trace 的 hook 还必须满足该内核调用点的实时和上下文限制。

## 应用 TLS 指针槽的拥有权

### 槽是每任务的指针数组

本版本每个任务有 N 个 `void *` 槽。setter/getter 只检查索引并写入或读取指针；它们不复制对象，不为对象加锁，也不登记分配器。`NULL` 任务句柄表示调用任务。索引越界时 setter 不写，getter 返回 `NULL`；合法但未安装对象的槽也可返回 `NULL`，因此错误索引不能靠返回值区分。[公开接口](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/include/task.h#L1910-L1926)、[实际实现](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L5937-L5987)

建议集中定义槽索引，例如 0 给解析器上下文，1 给错误记录；库不能各自假定自己独占槽 0。增加一个槽会增加每个启用该配置的 TCB 的指针存储，预算要按全部任务算，包括系统服务任务。精确 TCB 大小仍需由目标编译结果确认，包含对齐和其他可选字段。

TLS 的“local”描述查找关联，不是内存访问隔离。两个任务完全可以把同一个地址装进各自槽；在本章非 MPU 条件下，槽本身不提供硬件访问保护。任务的局部栈变量若被地址传给别的任务，也不因此安全。不能用 TLS 代替对象级同步。

### 指针槽没有自动析构合同

核对这个固定版本的 `task.h`、`tasks.c` 和删除路径，没有应用 TLS 槽的逐槽删除回调注册接口，也没有自动遍历并释放这些槽所指对象的步骤。不得把厂商分支或其他版本的类似名称 API 写成本版本保证。下节的 C-runtime TLS 清理宏是另一套接口。[槽实现](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L5937-L5987)、[TCB 回收](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L6401-L6453)

应把每个槽的合同写清：谁分配，是否共享，谁能换指针，何时所有使用者停止，以及谁按哪种分配器释放。把槽设为 `NULL` 只移除这一处引用，不会撤销其他任务、ISR 或 DMA 已持有的引用。getter/setter 在此版本也没有把“指针发布、对象初始化、读者引用计数和删除”组成一个原子事务；即使目标上的单次指针写是不可分割的，整体协议仍需独立证明。

### 原创退出协议示例

以下是应用设计伪代码，没有编译或执行；不是内核代码。选择每个 worker 独占的解析上下文：

```text
worker 入口
  分配并完整初始化自己的 context
  失败时报告创建失败并进入约定的退出路径
  在自己的 TLS 槽安装 context
  处理请求；外部通过消息提出停止
停止路径
  停止接受新工作
  按驱动合同等待已在途 DMA 和回调停止使用 context
  结束共享引用并归还持有的应用资源
  清空自己的槽
  使用匹配的分配器释放 context
  发布应用资源已清理的确认
  删除自己；不再返回任务入口
```

“资源已清理”确认不等于“TCB 与任务栈已经回收”。自删后的内核清理还需要 idle 获得运行机会。管理者不能看到确认就复用静态任务栈/TCB，也不能再凭旧句柄查询状态；静态存储复用另需明确的内核回收完成条件。直接从别的任务强制删除 worker 会绕开上述应用退出步骤。[自删与回收路径复习](kernel-task-lifecycle.md)

## C 运行时 TLS 与 Newlib

### V11.1.0 的实际适配路径

本版本 `configUSE_NEWLIB_REENTRANT=1` 会包含 `newlib-freertos.h`。该头启用 `configUSE_C_RUNTIME_TLS_SUPPORT`，默认把 TLS block 定义为 `struct _reent`，并提供三类适配：初始化调用 `_REENT_INIT_PTR`，切换当前块时更新 `_impure_ptr`，销毁调用 `_reclaim_reent`。TCB 中相应字段是 `xTLSBlock`，不应照搬旧版本的内部字段名。[Newlib 适配头](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/include/newlib-freertos.h#L32-L62)、[必需宏检查](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/include/FreeRTOS.h#L109-L153)

跟踪三个实际时刻：

1. 创建时初始化新任务的 block
2. 调度启动和上下文切换时设置当前任务的 block
3. `prvDeleteTCB` 回收时销毁被删任务的 block

第 3 步发生在实际回收上下文；自删任务的清理可能由 idle 执行。自定义销毁适配要使用传入的被删任务 block，不能依靠“当前任务”推断清理对象，也不能随意阻塞或做长时间 I/O。[初始化](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L1932-L1938)、[启动](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L3712-L3719)、[切换](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L5129-L5135)、[销毁](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L6401-L6416)

本版单核切换路径先执行 `traceTASK_SWITCHED_IN`，随后才调用 `configSET_TLS_BLOCK`；因此该 trace 调用点不能仅凭当前 TCB 已更新，就假定 libc 的当前重入块也已指向新任务。切换仍在 port 的异常上下文中完成。[切换次序](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L5108-L5135)、[PendSV 路径](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/portable/GCC/ARM_CM4F/port.c#L505-L544)

### 每任务重入状态不等于整套 libc 已线程安全

Newlib 的 `_reent` 关联每执行上下文的库状态；全局内存池和共享 I/O 仍有各自合同。官方手册要求多线程下提供适当的 malloc 锁，并说明 malloc 的锁调用可能递归。按实际 Newlib 构建核对 `__malloc_lock`/`__malloc_unlock`，以及它是否采用 retargetable locking 接口；不要假定链接成功就装上了可用于多任务的锁。[Newlib 重入状态](https://sourceware.org/newlib/libc.html#Reentrancy)、[malloc 锁](https://sourceware.org/newlib/libc.html#g_t_005f_005fmalloc_005flock)、[可重定向锁](https://sourceware.org/newlib/libc.html#g_t_005f_005fretarget_005flock_005finit)

集成清单还应包含：

- libc 的 heap 与 `pvPortMalloc` 所选 heap 是否独立；对象保持明确的分配/释放接口归属，不得在未证明分配器兼容和同步合同的情况下混用 `free` 与 `vPortFree`。`heap_3` 最终委托 libc 的 `malloc/free`，仍需核对直接 libc 调用与 FreeRTOS 包装路径共同使用的锁。[heap_3 分配与释放](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/portable/MemMang/heap_3.c#L59-L93)
- 系统调用桩和共享设备驱动是否序列化；两个 `printf` 调用的业务记录是否可能交错
- 库锁是否允许同任务递归；调度启动前及 idle 清理时能否安全使用；锁内是否触发再次分配或日志
- ISR 是否会重入 libc；不能仅靠切换任务时更新 `_impure_ptr` 就推断 ISR 拥有独立重入块，更不能在 ISR 等待普通任务互斥量
- Newlib 版本、newlib-nano 等构建选项、内存峰值和清理路径是否有目标证据

即使任务 TCB 和栈都是静态存储，库调用仍可能另外申请动态内存。`_reclaim_reent` 负责该适配的库状态清理，不负责释放应用装入指针槽的任意对象。内核适配头本身也提醒需要提供系统桩和 malloc 锁。

### 与语言 TLS 的边界

应用 `void *` 槽、Newlib 重入块、C11 `_Thread_local` 和 C++ `thread_local` 不能只因都叫 TLS 就互相替代。语言级变量的地址计算、初始化和销毁依赖实际工具链与运行时适配；打开 N 个槽不会自动给它们建立这套合同。GCC 官方 TLS 文档也明确指出其 TLS 扩展需要系统支持，不能假定每个平台都可用。该文档讲的是 GNU `__thread`，不能把其中所有限制直接当成 C++ `thread_local` 的语言规则。[GCC TLS 文档](https://gcc.gnu.org/onlinedocs/gcc/Thread-Local.html)

若项目需要语言 TLS，应在目标构建上分别验证两个任务的实例地址和独立更新、首次初始化、删除后的析构及再创建；本章没有完成这些验证。

## 运行时间统计究竟量了什么

### 单位来自应用选择的计数源

`portCONFIGURE_TIMER_FOR_RUN_TIME_STATS` 建立统计源，读取宏提供当前数值。该值可以来自合适的自由运行硬件定时器，也可以来自另行核对的周期计数器或软件扩展时钟。单位可以是计数刻度、微秒或 CPU 周期，不能统一写成“CPU 周期”。自由运行外设的寄存器可直接读取，机制不要求每个统计刻度都触发一个定时器 ISR；需要溢出扩展时才可能设计相应中断。[统计源合同](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/include/FreeRTOS.h#L2664-L2676)

本版本在任务切出时读取源，按本次源值与上次切换采样值的差额累加任务计数，再选下一个任务。`uxTaskGetSystemState` 复制已存的任务计数，并读取总源值；它不会为当前仍运行的任务补记尚未切出的整段时间。[单核计账](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L5068-L5097)、[快照读取](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L4383-L4457)

若源在 ISR 中仍继续计数，这个切换区间也包含期间的 ISR 时间；该路径没有单独减去所有 ISR 开销。因此报表是按任务切换区间归属的运行时间，不是逐指令的任务纯执行时间。切换、trace 和采样开销也受采样点影响。需要 ISR 独立成本或端到端截止时间时，另设测量，不能从运行占比直接推出。

### 精度和跨度要一起选

固定头文件建议统计源频率至少为 tick 的十倍。更高频能减少短运行片段的量化，但增加相同位宽源的回绕风险。记录源的实际频率、读开销、休眠和调试暂停时是否继续计数，以及 CPU 变频时单位是否改变。[频率与分辨率说明](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/include/task.h#L2299-L2317)

原创算例：32 位统计源若是 1 MHz，回绕周期是 `2^32 / 1,000,000` 秒，约 71.58 分钟；若选 200 MHz 周期源，则约 21.47 秒。这两项都不是 32 位 RTOS tick 的回绕周期。20 µs 运行片段在 10 kHz 源下只有 0.2 个刻度，单个片段可能看不到变化；在 1 MHz 源下约 20 个刻度，但仍有边界量化和读开销。

### V11.1.0 没有自动修复统计源回绕

单核切出路径只有当前源值大于上次采样值时才累加；源码明确提醒没有溢出保护。源从大数回到小数的那次区间不会按模减法补回。把报表采得更频繁也不会自动修复切换计账中已经漏掉的区间。[回绕判断](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L5075-L5097)

`configRUN_TIME_COUNTER_TYPE` 默认是 `uint32_t`。改成 `uint64_t` 会扩大软件字段，不能让一个只返回 32 位值的硬件源自动变成 64 位单调源。要长期测量，需要实际提供一致读取的扩展计数，并一起检查源范围、任务累加字段和算术中间值。32 位 MCU 上的多字读取也需要保护或一致性协议；本 port 的 `portTICK_TYPE_IS_ATOMIC` 是 32 位 tick 的合同，不是任意 64 位运行计数的保证。[计数类型](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/include/FreeRTOS.h#L2839-L2845)、[tick 类型](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/portable/GCC/ARM_CM4F/portmacro.h#L62-L77)

累加字段本身也会溢出。即使扩展源够宽，若任务累加字段或百分比中间乘法过窄，报表仍会失真。不要在调度已开始后擅自把源归零；那会与已累积的任务计数和切入基准不一致。

### 占比要说明窗口和分母

累计占比回答“统计起点以来占了多少”，不回答“最近一秒是不是超载”。窗口占比需要两次样本相减，确认同一任务实例、同一源和连续有效窗口，再用窗口总量作分母；遇到删除再创建、源复位或未处理回绕时应标为无效，而不是继续减。

本版本便利格式化把累计总量先除以 100 再做整数除法，会有量化；累计总源值不足 100 时除后为零，内核路径会跳过百分比输出。自定义窗口报表也要保护除零，扩大乘法中间值，并声明舍入方式。总源的新样本、尚未切出的区间、已删除任务不在当前表中以及整数舍入，都可能使各行合计不等于 100%。不要强行归一化掩盖这些差异。[格式化路径](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L7439-L7458)

## 快照与追踪也会扰动被测系统

### 状态值和字符串有不同寿命

`uxTaskGetSystemState` 挂起调度器以遍历任务，但 ISR 仍可能运行；恢复后任务状态可以立刻改变。`eReady` 只描述采样时资格，不能授权调用者接下来强删、复用或操作任务。`TaskStatus_t.pcTaskName` 是指向 TCB 内名字的借用指针，任务删除后可能失效；`xHandle` 和栈地址也不应当成长期拥有的资源。[快照合同](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/include/task.h#L158-L175)、[采样路径](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L4383-L4457)

先读任务数量、再分配数组、最后采样，期间任务可能增加；数组过小时函数返回零。只处理实际返回的行，并把零视为本次采样失败，而不是空系统。在高频创建场景，应使用有上界的容量和重试策略，或通过应用任务管理协议稳定参与者，不能无限重试影响实时任务。

安全导出的一种应用设计是：所有应用创建/删除都遵守同一个寿命协议，采样者在该协议的保护期内取得快照并把名字复制到自己的固定缓冲，再释放保护并异步格式化。系统任务寿命另行确认。不能在快照返回后无保护地逐个复制借用名字，认为“数据已经在数组里”就安全；协议必须覆盖那段复制期间。

### 原始数据仍有扫描成本

去掉字符串格式化可以省掉临时分配和 `snprintf`，但不能声称原始快照是零成本：此版本的列表遍历调用 `vTaskGetInfo` 时要求检查栈空闲量，会扫描填充区域；开 trace 也会影响新栈的初始化。单任务 `vTaskGetInfo` 可选择不扫栈，但寿命保护仍由调用者承担。[列表采样与栈扫描](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L6241-L6300)、[新栈填充条件](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L129-L134)

文档中的笼统时延警告要结合实现阅读：原始快照主体是挂起调度器，并非整段关掉所有中断；内部部分步骤仍有临界区。生产设计应分别测“多久任务不能切换”“多久相应中断被屏蔽”“格式化任务花了多久”。不能把三个时间混成一个数。

### trace 回调不能随意打印

trace 宏在实际调用点展开；它不是自动排队到一个低优先级日志任务。切换、API 或 ISR 路径上的记录器应短小、有明确上界，避免阻塞、malloc 和同步串口输出。推荐设计是复制固定长度事件到已验证的有界缓冲，由普通任务消费；缓冲满时按约定丢弃并计数。记录器本身若也会被 ISR/任务并发访问，仍需独立证明，不能随手套一个未经验证的无锁结构。

比较关闭与打开 trace 时的延迟、栈和丢事件量；“打开诊断后才偶尔超时”可能是诊断成本改变了系统。采样频率、任务数、栈扫描长度、hook 工作量和中断负载都应进入验收条件。

## 另外三个容易误读的字段

### 存储来源标记不是一个通用布尔值

尽管成员旁的简短注释用了真假措辞，V11.1.0 的内部 discriminator 有三个常量：动态栈加动态 TCB 为 0，静态栈加动态 TCB 为 1，静态栈加静态 TCB 为 2。删除据此决定释放两者、只释放 TCB 或不释放两者。[定义](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L115-L125)、[回收分支](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L6417-L6453)

本章非 MPU 的普通创建组合通常只有全动态与全静态；中间组合来自其他受支持的创建/MPU 配置，不能假装本章 port 会走全部路径。该字段是否存在也受配置控制。内核存储归属与 TLS 所指应用对象归属是两份合同。[配置组合](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/include/FreeRTOS.h#L3039-L3072)

### `ucDelayAborted` 不是应用取消令牌

`xTaskAbortDelay` 可以让当前处于 Blocked 的任务提前 Ready；有事件等待节点时，路径设置该标记，内部超时检查消耗它以避免按剩余预算再次阻塞。进入新的延时列表时也会清除标记。因此它不是持续有效的“任务要退出”状态，也不是所有阻塞方式共享的业务取消返回值。应用仍需要停止消息、停止标志和资源清理协议。[解除阻塞](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L4574-L4667)、[超时检查消费标记](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L5550-L5558)、[新等待清除](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L8424-L8429)

### `volatile` 与“栈顶”不能靠字面推断

通知值和通知状态在 TCB 中带 `volatile`，但通知路径仍用任务临界区及 FromISR 的屏蔽入口保护检查、更新和唤醒。字段有 `volatile` 不建立跨字段原子事务，也不替代正确的 ISR 优先级合同。[通知等待保护](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L7731-L7752)、[任务更新](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L7848-L7863)、[ISR 更新](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L7985-L8007)

本章 ARM_CM4F 的 `portSTACK_GROWTH=-1`，任务栈向低地址增长。栈分配基址、初始化时高端位置和某次保存的栈指针不是同一个概念；高水位单位是 `StackType_t` 项，并非直接的字节。看调试器地址时先核对 port，不要写成 Cortex-M 栈向高地址增长。[port 类型与方向](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/portable/GCC/ARM_CM4F/portmacro.h#L49-L82)、[初始栈顶](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L1828-L1848)

## 原创算术模型与已执行测试

下例只模拟统计源样本、有限宽度累加和报表算术，不模拟任务调度、硬件计数读或 FreeRTOS API。用 8 位值演示回绕，不改变本章 32 位 tick 条件。Python 整数在模型中显式取模；移植成 C 时需重新核对类型和中间值宽度。

```python
def account_segment(previous, now, accumulated, bits):
    limit = 1 << bits
    assert 0 <= previous < limit and 0 <= now < limit
    assert 0 <= accumulated < limit
    accepted = now - previous if now > previous else 0
    return (accumulated + accepted) % limit, accepted


def modulo_elapsed(previous, now, bits):
    # 仅在已知真实间隔小于一个回绕周期时，才可解释为真实差额。
    return (now - previous) % (1 << bits)


def window_permille(task_delta, total_delta):
    assert task_delta >= 0 and total_delta >= 0
    return None if total_delta == 0 else task_delta * 1000 // total_delta


def quantized_segment(start_us, end_us, counts_per_second):
    return (end_us * counts_per_second // 1_000_000
            - start_us * counts_per_second // 1_000_000)
```

此同一代码已在主机运行七项单元测试，并穷举 8 位的 65,536 对前后样本；全部通过。检查内容为：不回绕累加、相同样本的零增量、跨回绕时上述守卫漏记、累加字段自身回绕、零分母和千分比截断、短片段量化、所有 8 位样本的算术关系。关键观察是 `previous=250, now=4` 时，上述计账模型接受 0，而限定单周期的模差额为 10。这是模型结果，不是内核运行日志。

## 动手练习与验收

以下内核和板级练习尚未执行：

1. 标签调用：注册一个无阻塞 hook，确认注册没有执行 hook；从管理任务显式调用，记录当前任务。再分析把调用接到切换 trace 会有什么限制。
2. TLS 退出：每个 worker 独占一个上下文；分别检查正常停止、初始化失败、外部强删和 idle 饥饿。验收没有泄漏、悬空引用或跨分配器释放，区分应用资源清理与内核存储回收。
3. Newlib：记录工具链与库构建选项、实际锁符号和桩函数；两个任务交错使用需要重入状态的库功能及分配器。压力测试通过只覆盖实际路径，不能证明所有 libc 调用或 ISR 用法安全。
4. 统计源：测实际频率、读开销、睡眠/调试/变频行为；让源跨回绕，检查原始数和累计数。验证扩展源时加入溢出与读取交错，不只看正常递增。
5. 快照：在受控创建/删除并发下检查数组不足、名字寿命、实例编号和失败重试；报表不得查询已失效句柄。
6. 诊断预算：分别开启 trace、运行计数、栈扫描和格式化，测任务切换延迟、ISR 屏蔽时间、栈峰值和记录缓冲满后的丢弃计数。
7. 解除等待：分别提前解除普通延时、队列等待和通知等待，记录 API 最终返回及应用停止条件；不把一个内核标记当成业务取消协议。

验收报告至少包含固定 commit、编译配置、目标 port、计数源与频率、任务数量、调用上下文、存储拥有者和实际覆盖的失败路径。源码核对、主机算术、内核运行与板级时延是四层不同证据。

## 面试追问

“打开 TLS 后，每个任务的数据就互不干扰了吗？”答：槽关联是独立的，但指向对象可以共享，访问隔离和对象同步没有自动建立。还要区分指针槽、libc 状态块与语言 TLS。

“任务运行占比之和为什么不是 100%？”答：先核对分母、当前未切出区间、删除的历史任务、源回绕和整数舍入，再检查测量口径。不能先强制归一化再解释。

“给 TCB 多加几个诊断字段会影响实时性吗？”答：字段增加每任务存储；启用功能还可能增加创建填栈、切换计账、库清理、快照扫描和 hook 开销。应分别度量，不能只算结构大小。

## 来源与许可

- [官方 V11.1.0 固定源码](https://github.com/FreeRTOS/FreeRTOS-Kernel/tree/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0)、[tasks.c](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c)、[task.h](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/include/task.h)：本章版本专属实现与接口合同。
- [Newlib 官方手册](https://sourceware.org/newlib/libc.html)、[GCC 官方 TLS 文档](https://gcc.gnu.org/onlinedocs/gcc/Thread-Local.html)：运行时集成背景；不代表项目所链接的具体库版本已经验证。
- [FreeRTOS-Kernel V11.1.0 MIT 许可](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/LICENSE.md)。相关内核文件标注 Copyright (C) 2021 Amazon.com, Inc. or its affiliates，SPDX-License-Identifier: MIT。本文没有复制内核源码段；若分发内核代码或其重要部分，应保留对应版权与 MIT 许可声明。

正文、退出协议、算例、模型与练习为原创教学内容。来源用于核对，不表示源码维护者为本讲义背书。SMP、MPU、其他 port、厂商扩展及其他内核版本需要重新验证。
