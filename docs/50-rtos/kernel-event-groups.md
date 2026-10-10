# 源码补充 · 事件组的条件判断、快照与延迟处理

建议先学 [RT05 · 队列、通知和事件组](rt05.md)，再读本章。本章是源码补充，不新增核心题编号。

## 直接回答

事件组保存一组共享条件位。任务可以等待其中任意一位成立，也可以等待指定各位全部成立；一次置位可以让多个等待者进入 Ready。位已经为 1 时再置 1，不会记录第二次事件，因此它适合可合并的条件，不适合保存每条消息或精确计数。

读源码时抓住三个区别：等待函数返回的位快照与对象当前值可能不同；解除阻塞与任务真正运行有时间差；ISR 提交置位请求与 daemon 真正修改事件组有时间差。把这三件事分开，清位、超时和延迟的推演才不会混在一起。

## 版本与适用范围

源码依据为官方 FreeRTOS-Kernel **V11.1.0**，对应 commit `dbf70559b27d39c1fdb68dfb9a32140b6a6777a0`。主要阅读 [event_groups.c](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/V11.1.0/event_groups.c)、[event_groups.h](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/V11.1.0/include/event_groups.h) 及它们调用的 [tasks.c](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/V11.1.0/tasks.c) 和 [timers.c](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/V11.1.0/timers.c)。

运行语境限定为单核 `configNUMBER_OF_CORES=1`、GCC `ARM_CM4F` 移植；本章数值例子采用 32 位 tick，业务只使用低 24 位。`EventBits_t` 取自 `TickType_t`，头文件保留最高 8 位供内核控制信息使用。其他 tick 宽度、SMP、MPU 移植需要重新核对，不能直接搬用本章位掩码和 ISR 路径。[固定头文件](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/V11.1.0/include/event_groups.h)、[固定移植头文件](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/V11.1.0/portable/GCC/ARM_CM4F/portmacro.h)

正文、推演和伪代码为原创教学内容，没有复制内核实现。下列练习尚未运行 FreeRTOS 内核、移植或板级实验；源码核读不能替代实时性和设备寿命的测量。

## 原理拆解

### 1. 先确定每一位表示什么

假设一个采集服务使用三位：

| 位 | 含义 | 建议清除者 |
| --- | --- | --- |
| `LINK_OK = 0x01` | 通信链路可用 | 管理链路状态的任务 |
| `CONFIG_OK = 0x02` | 当前配置已经验证 | 管理配置状态的任务 |
| `SAMPLE_READY = 0x04` | 至少有一批样本可处理 | 约定的消费路径 |

前两位是持续状态，第三位允许合并。若每批样本都必须处理，还需要队列或另一个有界存储，事件位只能提示去检查它。位变为 1 也不自动保证 DMA 已停止、缓冲仍有效或某个任务拥有数据；这些是应用协议的一部分。相关数据合同见 [RT05](rt05.md) 和 [队列等待路径](kernel-queue-waits.md)。

`EventGroup_t` 的核心字段是当前位图 `uxEventBits` 和等待者链表 `xTasksWaitingForBits`。事件组有自己的实现，不由修改 `Queue_t` 配置生成。trace 字段和静态分配标记受配置影响，读结构体时应沿着条件编译看实际布局。[event_groups.c](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/V11.1.0/event_groups.c)

### 2. AND 与 OR 是针对掩码的两个谓词

设返回位值为 `bits`，关心的位为非零掩码 `mask`：

- OR 等待：`(bits & mask) != 0`，指定集合中至少一位为 1
- AND 等待：`(bits & mask) == mask`，指定集合中各位都为 1

`xEventGroupWaitBits()` 的 `xWaitForAllBits` 选择这两个谓词。它不要求整个事件组等于 `mask`；无关位可以同时为 1。`xTicksToWait=0` 表示仅检查，不阻塞。需要无限等待时，`portMAX_DELAY` 的无限语义还依赖 `INCLUDE_vTaskSuspend=1`，不要只看常量名称。[event_groups.h 的 WaitBits 契约](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/V11.1.0/include/event_groups.h)

例如等 `LINK_OK | CONFIG_OK`：OR 只证明链路或配置至少一项就绪，不能据此开始需要两者的采集；AND 才检查这两个条件一起成立。

### 3. 返回值是快照，清位按等待掩码执行

`xEventGroupWaitBits()` 返回 `EventBits_t`，没有独立的“成功/超时”状态码。应用要对返回值使用自己的 OR/AND 谓词。返回的是整个业务位图快照，可能包含不在等待掩码中的位。

若进入函数时条件已经成立，返回值先记下当前位图。`xClearOnExit=pdTRUE` 时，再把等待掩码覆盖的位从对象中清除。因而返回值仍能说明刚才观察到了什么，之后调用 `xEventGroupGetBits()` 可能看到不同值。

若任务原来处于 Blocked，置位路径把匹配时的快照存入该任务的事件链表项，再解除阻塞。它恢复执行时取回这个快照，并剔除内核控制位。快照不是等待任务重新调度时再读的一张照片。[WaitBits 与 SetBits 实现](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/V11.1.0/event_groups.c)

清除范围是 `uxBitsToWaitFor` 指定的位，不是只清其中“最先发生”的一位，也不会清除无关业务位。例如 OR 等待 `0x03`，匹配时位图为 `0x07`，返回 `0x07`，请求清位后对象保留 `0x04`。若匹配时只有 `0x01`，清 `0x03` 中本来为 0 的位不会多产生一个事件。

持续状态一般不应由每个观察者都自动清除。多个模块共同观察 `LINK_OK` 时，可由链路管理任务负责置位和清位，观察者采用不清位等待。是否清位来自业务含义，不是固定使用 `pdTRUE` 的口诀。

### 4. 一次置位会检查全部等待者

V11.1.0 的 `xEventGroupSetBits()` 先更新对象位图，再遍历等待者。每个满足自己条件的任务都得到匹配快照；需要自动清除的掩码被汇总，遍历结束后才统一清位。

因此两个已经阻塞、都等同一位且要求清位的任务，可以被同一次置位唤醒。不能推演成“第一个任务把位拿走，第二个必然看不到”。它也没有互斥锁的独占所有者语义。具体运行顺序仍由任务调度决定，Ready 不等于已经执行。[SetBits 实现](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/V11.1.0/event_groups.c)、[RT01 · 调度基础](rt01.md)

要区分“已经阻塞的多个等待者”和“清位完成后才来检查的新任务”。后者只能检查那时的对象值，内核不会把之前快照补发给它。等待条件设计不能靠所有任务碰巧同时到达。

## 例子与推演

### 推演一 多等待者共享快照

初始位图为 0，三个任务已经阻塞：

| 任务 | 掩码 | 等待方式 | 自动清位 |
| --- | --- | --- | --- |
| A | `0x03` | AND | 是 |
| B | `0x01` | OR | 是 |
| C | `0x04` | OR | 否 |

某任务一次置 `0x07`：

1. 位图变成 `0x07`
2. A 的 AND、B 的 OR、C 的 OR 都成立，三个任务保存的业务快照都是 `0x07`
3. 自动清位掩码合并为 `0x03`，对象剩下 `0x04`
4. A、B、C 何时真正运行由调度决定；它们各自的返回快照仍是 `0x07`
5. D 若此后才以零等待时间检查 `0x01`，看到的是对象当前值 `0x04`，谓词不成立

本推演限定没有其他代码在步骤间改位。若解除阻塞的更高优先级任务在 `xEventGroupSetBits()` 恢复调度后改位，**SetBits 自己的返回值**还可能随之变化；它不是“此次输入掩码一定留在对象中”的证明。[SetBits 返回值契约](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/V11.1.0/include/event_groups.h)

### 推演二 同一位的两次置位会合并

生产者先后两次置 `SAMPLE_READY`，期间没有清位。位图仅表示“这一位为 1”，不能从返回值推出已经完成两批。消费者若只处理一批就清位，另一批可能仍在缓冲中却没有新提示。

一个可选设计是：队列保存批次，事件位提示去检查队列，消费者按约定检查到无待处理工作再完成同步。队列空与清位之间仍需处理并发窗口；单靠普通变量顺序无法证明不会丢提示。若只有一个目标消费者，优先比较直接任务通知或队列自身阻塞是否已经满足需求。[RT05](rt05.md)

## 源码路径与超时边界

### 1. 等待登记同时涉及事件项与状态项

在非零等待且条件尚未成立时，WaitBits 挂起调度器，使用 `vTaskPlaceOnUnorderedEventList()`：

- `xEventListItem` 登记掩码及本次等待的清位、AND/OR 控制信息，加入事件组等待链表
- `xStateListItem` 通过任务阻塞路径进入延时链表，或进入配置允许的无限等待容器

同一任务用两个不同链表项表达“在等什么”与“最迟等到什么时候”。链表项的当前 container 必须唯一，详见 [内核链表](kernel-lists.md) 和 [任务生命周期](kernel-task-lifecycle.md)。这些内部函数不是应用 API。[tasks.c：unordered event-list 路径](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/V11.1.0/tasks.c)

事件匹配时，`vTaskRemoveFromUnorderedEventList()` 在调度器挂起的前提下存入匹配快照，移除事件项和状态项，直接加入相应 Ready 链表，并在单核分支记录必要的延后调度请求。不要把它误写成所有情况都先进入 `xPendingReadyList`；队列的另一条解除阻塞路径有自己的调度器状态判断。[同版本 tasks.c](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/V11.1.0/tasks.c)、[队列等待路径](kernel-queue-waits.md)

### 2. 超时离开 Blocked 后还要重新检查

tick 超时路径会把任务从延时容器移出，必要时也移除仍挂着的事件项，再放进 Ready。任务可能继续等待 CPU。它真正返回 WaitBits 时，如果没有“因位匹配解除阻塞”的内部标记，会在临界区重新读取对象位图并检查条件；此时条件若已经成立，仍按 `xClearOnExit` 清除等待掩码。[tasks.c：xTaskIncrementTick](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/V11.1.0/tasks.c)、[event_groups.c：WaitBits 恢复路径](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/V11.1.0/event_groups.c)

三个有意分开的交错：

1. **先匹配再运行**：位在截止 tick 前置好，任务获得匹配快照；后来运行时对象可能已经被清除，返回快照仍支持判断条件
2. **先超时，再置位，再运行**：任务先由 tick 放回 Ready，之后其他任务置位，它恢复 WaitBits 时复查可能看见成立条件并清位
3. **先超时并完成复查，再置位**：返回快照不满足条件；随后发生的新置位不会改写已返回的值

所以“返回的位满足条件”表示本次等待路径观察到了条件，不能证明置位严格发生在最初截止时刻之前。需要严格截止时间的业务，应另外携带发生时间、序号和应用截止判定。反过来，非零返回值也不自动成功：AND 等 `0x03` 返回 `0x01` 仍不满足条件。

下面只表示应用如何判定结果，不是内核实现或完整可运行程序：

```text
snapshot = 等待(mask, 按业务选择清位, AND或OR, 最长阻塞tick)
matched = AND ? ((snapshot & mask) == mask)
              : ((snapshot & mask) != 0)
若 matched：继续验证本次会话和数据所有权，再处理工作
否则：按应用的超时或关闭协议处理，不访问已释放对象
```

零快照还可能来自事件组被删除后对阻塞任务的解除阻塞，不能单凭它区分删除和普通等待失败。严格原因区分需要应用状态；对象存活是所有上述复查的前提。[Delete 契约](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/V11.1.0/include/event_groups.h)

## WaitBits 与 Sync 的区别

`xEventGroupWaitBits()` 观察条件，不替调用者贡献某一位。`xEventGroupSync()` 把“贡献自己的位”和“等待所有集合位”作为同步操作，用于任务会合；成功会合会自动清除等待集合，返回清除前的快照。它没有 OR 选择，也不能在 ISR 调用。[Sync 契约与实现](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/V11.1.0/include/event_groups.h)、[event_groups.c](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/V11.1.0/event_groups.c)

设 A、B、C 分别贡献 `0x01`、`0x02`、`0x04`，都等待 `0x07`。A、B 先到会合点并阻塞；C 最后到达时，前两个等待者可能已被置位路径唤醒和清位。Sync 使用置位前记录的值与 C 的贡献进行本次会合判定，让最后参与者也能看到完整的会合快照。把它随手拆成 SetBits 后再 WaitBits，不能保留这个保证。

会合成功只证明参与者报告到达了约定点。一个参与者超时、退出或开始新会话时，需要定义其他参与者怎样取消、重试或切换轮次；未完成会合的贡献位不能被无条件当作下一轮的新到达。事件组没有自动附加轮次编号、错误传播和资源释放协议。

## ISR 延迟处理与配置边界

### 1. 普通置位函数不能从 ISR 调用

`xEventGroupSetBits()` 要检查等待者链表并可能解除多个任务的阻塞，工作量随等待者数量变化。固定版本公开 API 明确禁止在中断中调用它。ISR 使用 `xEventGroupSetBitsFromISR()`，该调用通过 `xTimerPendFunctionCallFromISR()` 向 **timer daemon 的同一命令队列**投递延迟函数调用，daemon 之后调用内部置位回调。没有单独的“事件组回调任务”或独立回调队列。[固定头文件与实现](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/V11.1.0/include/event_groups.h)、[timer 延迟函数路径](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/V11.1.0/timers.c)

`pdPASS` 表示请求已成功入队。队列满会失败，ISR 不能等待有空位；入队成功也不表示事件位已经修改，更不表示等待任务已处理完。初始化并检查 `pxHigherPriorityTaskWoken`，再按移植要求请求切换；此标志针对投递时可能唤醒的 daemon，不是事件组目标任务已经完成工作的确认。[官方 FromISR 文档](https://www.freertos.org/Documentation/02-Kernel/04-API-references/12-Event-groups-or-flags/06-xEventGroupSetBitsFromISR)

另一组入口也要分别看合同：`xEventGroupClearBitsFromISR()` 同样提交延迟清位，但它没有 `pxHigherPriorityTaskWoken` 参数；V11.1.0 头文件要求成功提交后执行适用的 `portYIELD_FROM_ISR(pdTRUE)`。`xEventGroupGetBitsFromISR()` 则是在 ISR 合规的保护路径下读取当前位快照。后者的读取不等待前面排队的置位命令执行。[event_groups.h](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/V11.1.0/include/event_groups.h)、[event_groups.c](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/V11.1.0/event_groups.c)

### 2. daemon 延迟属于端到端时延

一条 ISR 置位通知至少要经过“ISR 投递→daemon 获得 CPU→处理置位→目标任务获得 CPU”。慢定时器回调、积压命令、更高优先级任务和屏蔽中断的区间，都可能影响实际响应。

调高 `configTIMER_TASK_PRIORITY` 可以改变 daemon 获得 CPU 的先后，但不会取消它前面正在执行的长回调，也不会让软件服务成为精确硬件触发器。为一个接收任务设计低延迟通知时，应先核对是否真的需要一对多条件位，再与任务通知、队列或信号量的合规 ISR 路径比较。[定时器服务任务](kernel-timer-daemon.md)、[RT06](rt06.md)

### 3. 配置检查应区分普通操作与延迟入口

| 检查项 | 本章采用的边界 |
| --- | --- |
| `configUSE_EVENT_GROUPS=1` | 编译事件组功能，包含 `event_groups.c` |
| 静态或动态分配开关 | 对应 `xEventGroupCreateStatic()` 或 `xEventGroupCreate()`；创建失败要处理 |
| `configUSE_TIMERS=1`、`INCLUDE_xTimerPendFunctionCall=1` | 事件组 ISR 延迟置/清位路径需要；普通任务上下文等待并不因此都依赖 timer daemon |
| timer 优先级、栈与命令容量 | 按所有共享服务工作计算，不能只统计业务定时器 |
| `configUSE_TRACE_FACILITY` | 影响 FromISR 入口是宏包装还是编译函数，不等于必须开启 trace 才能使用它 |
| Cortex-M ISR 优先级 | 遵守 GCC ARM_CM4F 的系统调用中断优先级上限与优先级分组合同 |

上述开关来自 [FreeRTOS.h](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/V11.1.0/include/FreeRTOS.h) 及 [event_groups.h](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/V11.1.0/include/event_groups.h)。ISR 优先级断言和屏蔽方式见 [固定 port.c](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/V11.1.0/portable/GCC/ARM_CM4F/port.c)；NVIC 数值方向及编码复习见 [RT06](rt06.md)。关掉断言不会放宽调用权限。

## 常见误区与失败边界

- **把位当计数器**：重复置同一位会合并，不能保留多次发生或输入顺序
- **把清位当消费锁**：多个已阻塞等待者可以得到同一快照，互斥访问还需另一个合同
- **根据非零返回值判断 AND 成功**：必须检验掩码内各位是否齐全
- **看见当前位已清就否定返回快照**：两者记录的是不同时间和用途
- **把最长阻塞 tick 当函数精确返回时刻**：Ready 后仍有调度等待，超时路径还会复查
- **在调度器已经挂起时请求阻塞**：WaitBits/Sync 的非零等待违反其检查边界，不能靠不发生切换来实现等待
- **忽略删除前已排队的 ISR 命令**：命令保存着事件组地址，之后置位回调仍可能访问旧对象

删除事件组会解除当前阻塞等待者，但不自动停止硬件生产者，也不取消 timer 队列里已经提交的延迟函数。关闭流程应先拒绝新操作，停止 ISR/定时器等生产入口，证明在途操作及延迟请求都不再访问旧对象，再删除和回收存储。具体完成确认由应用设计，不能把“FromISR 返回成功”或“把公共句柄设为 NULL”当作释放安全证明。[对象关闭场景 IC02](../120-interviews/ic02.md)、[定时器关闭边界](kernel-timer-daemon.md)

## 面试追问

1. **两个任务都清同一位，为什么两者都醒？** 说明 SetBits 遍历时使用同一更新后的位图，清除掩码在遍历结束后合并执行；随后到来的新等待者不在这个快照集合里。
2. **已经超时，为什么返回值还满足 AND？** 说明从 Blocked 到 Ready 和恢复 WaitBits 的区别，再追到超时路径的当前位复查；严格截止需要额外时间证据。
3. **ISR 入队成功，目标任务为何还没醒？** 分开检查命令提交、daemon 调度、共享回调耗时和目标任务调度，不只调目标任务优先级。
4. **删组可以当广播 stop 吗？** 先定义关闭状态与对象寿命；零快照不足以表达完整退出原因，删除也不能取消在途旧句柄访问。

## 动手练习与验收 尚未执行

### 练习一 位谓词与快照

枚举位图 `0x00` 到 `0x07`，对掩码 `0x03` 分别计算 OR/AND，记录自动清位后的对象值和等待返回值。再加入两个同位阻塞等待者及一个清位后才到的新等待者。

验收：AND 在 `0x01`、`0x02` 时不成立；`0x07` 的无关 `0x04` 不被清；两个原有匹配者收到快照；新等待者不能追认过去的置位。表格演算只检查这个模型，不证明并发内核实现。

### 练习二 超时交错

用事件序列分别安排“匹配→Ready→运行”“超时→置位→运行”“超时→运行复查→置位”。记录事件项和状态项所属容器、匹配标记、返回快照及对象值。

验收：任务不会在 Ready 后仍留在旧等待容器；复查时可能观察到超时之后的新位；应用不把快照当精确发生时间。若验证实际内核，应另建固定版本运行环境并保留可复现 trace，本章没有这样的测试结果。

### 练习三 共享 daemon 与关闭

设计两个软件定时器和一个 ISR 置位源，列出命令队列满、慢回调、关闭时延迟置位尚未处理三种场景。写出每种场景的失败计数、恢复策略和资源可释放条件。

验收：ISR 失败有可观测统计；成功入队和执行完成使用不同证据；没有命令能在事件组释放后继续引用它；关闭超时进入安全失败路径，而不是强行释放。板级最坏时延需要单独测量。

## 先修与复习入口

- [源码阅读路线](source-reading.md)：固定版本、配置和证据边界
- [内核链表](kernel-lists.md)、[任务生命周期](kernel-task-lifecycle.md)：等待容器和 Ready 转换
- [队列等待路径](kernel-queue-waits.md)：与事件组 unordered 等待路径对照
- [RT01](rt01.md)、[RT02](rt02.md)、[RT05](rt05.md)、[RT06](rt06.md)、[RT10](rt10.md)：先用 API 语义回答，再展开源码
- [定时器服务任务](kernel-timer-daemon.md)、[IC02](../120-interviews/ic02.md)：共享服务能力与关闭协议

## 来源与证据边界

- [V11.1.0 event_groups.c](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/V11.1.0/event_groups.c)：位图、等待登记、匹配者扫描、统一清位、超时复查和删除路径
- [V11.1.0 event_groups.h](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/V11.1.0/include/event_groups.h)：公开 API、控制位、返回值及 ISR 包装
- [V11.1.0 tasks.c](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/V11.1.0/tasks.c)：unordered 事件链表与 tick 超时移除
- [V11.1.0 timers.c](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/V11.1.0/timers.c)：延迟函数与 timer 命令共享 daemon 队列
- [官方 FromISR 说明](https://www.freertos.org/Documentation/02-Kernel/04-API-references/12-Event-groups-or-flags/06-xEventGroupSetBitsFromISR)：公开文档于 2026-10-10 核对，页面会更新，精确源码事实以本章固定版本为准

源码事实、原创设计建议和未执行练习已分别标明。引用用于核对机制，不表示官方为本章设计背书；其他版本须重新核对。
