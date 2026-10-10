# FreeRTOS 任务生命周期 · 从 TCB 到阻塞与唤醒

学习优先级：P1。先读 [RT01](rt01.md)、[RT02](rt02.md) 和 [内核链表](kernel-lists.md)。本章使用官方 FreeRTOS Kernel V11.1.0 固定提交 `dbf70559b27d39c1fdb68dfb9a32140b6a6777a0`，限定单核 GCC ARM_CM4F；默认阅读配置见 [源码导读](source-reading.md)。本章未执行内核、模拟器或板级实验。

## 直接回答

任务的运行资格由 TCB、当前任务指针和若干列表共同表达。`xStateListItem` 记录 Ready、延时等待或挂起等容器关系；`xEventListItem` 记录某个内核对象上的等待关系。一个有限超时的队列等待通常同时占用这两个节点，分别等待事件和截止时刻。先被内核处理的解阻塞路径会撤销相应等待关系，再使任务具备运行资格；API 成功与否仍由其返回路径决定。

Running 没有一张独立的普通“运行列表”：在本章单核主线中，运行任务仍在其优先级 Ready 列表，`pxCurrentTCB` 指出当前执行者。Ready 是资格，Running 是当前选择。[TCB 与当前任务](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L358-L464)、[单核选任务](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L159-L242)

## 原理拆解

### 1. TCB 只保留本次阅读需要的字段

| 字段 | 本章用途 | 配置边界 |
| --- | --- | --- |
| `pxTopOfStack` | 保存任务栈现场位置；本版要求它是首字段 | 保存/恢复布局由 port 决定 |
| `pxStack` | 栈存储起点，供初始化和回收 | 栈增长方向、静态/动态来源要核对 |
| `xStateListItem` | 任务的状态容器关系 | 删除、挂起相关表由功能配置决定 |
| `xEventListItem` | 对象等待或暂存的事件节点 | 值的含义由使用它的模块决定 |
| `uxPriority` | 当前有效任务优先级 | 与 NVIC 数字方向分开 |
| `uxBasePriority`、`uxMutexesHeld` | 互斥量继承前的基础优先级与持锁计数 | `configUSE_MUTEXES=1` 才存在 |
| 通知值与通知状态数组 | 指定任务的通知信息 | 由通知相关配置决定 |
| 静态分配标记等 | 区分哪些存储可由内核释放 | 条件编译，不能固定整个 TCB 的大小 |

这里不列完整结构，不把调试看到的偏移当成应用 ABI。尤其不能根据事件节点为空，就断定任务一定不是 Blocked：任务通知有自己的等待状态。[TCB 定义](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L358-L440)、[状态查询的通知检查](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L2485-L2610)

### 2. 创建成功到首次运行

沿 `xTaskCreate` 看三个阶段：获得 TCB/栈存储，初始化任务现场，加入 Ready。静态创建使用应用提供的存储；具体可用 API 与内存回收由配置决定。初始化时给两个节点设置 owner，把普通事件值设为 `configMAX_PRIORITIES - uxPriority`，再通过 port 构造初始栈现场。[创建与初始化](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L1718-L1752)、[节点初始化](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L1903-L1989)

创建前没有任务时，第一次加入会初始化任务列表。调度器尚未运行时，当前任务候选随创建的优先级更新；调度器已经运行时，新建高优先级任务可能触发抢占。因此创建者不能假定“任务要等我下一行执行完才开始”，参数对象必须先准备好。[加入 Ready](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L2017-L2093)、[RT10 · 参数寿命](rt10.md)

`vTaskStartScheduler` 创建 idle task，启用软件定时器时还创建服务任务；成功后设置调度状态和初始 tick，进入 `xPortStartScheduler`。本 port 设置异常优先级、tick 来源与 FPU 支持，经 SVC 启动首个任务；SVC 从当前 TCB 恢复现场并使用 PSP。初始参数以指针值进入任务现场，不复制其目标对象。[内核启动](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L3665-L3765)、[本 port 初始化与 SVC](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/portable/GCC/ARM_CM4F/port.c#L202-L231)、[首次启动](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/portable/GCC/ARM_CM4F/port.c#L260-L452)

启动所需对象创建失败与某个用户任务创建失败都要有错误路径。任务入口不能随意 return；本 port 的默认返回路径会进入错误处理。栈、异常自动保存和 port 软件保存的关系复习 [AR07](../30-architecture/ar07.md)。

### 3. 用容器表描述等待

以下表格仅描述普通队列、延时及显式挂起的主线，不替代通知或事件组的完整状态规则。表中“空”指 container 为 NULL。

| 观察状态 | 状态节点容器 | 事件节点容器 |
| --- | --- | --- |
| Ready / Running | 对应优先级 Ready 列表 | 通常为空 |
| 正数 `vTaskDelay` | 当前或溢出延时表 | 空 |
| 队列有限等待 | 当前或溢出延时表 | Q 的发送或接收等待表 |
| 队列无限等待，且支持无限阻塞 | suspended 表 | Q 的等待表 |
| 显式 `vTaskSuspend` | suspended 表 | 空 |
| 有序事件唤醒被暂存 | 原延时或 suspended 表尚未迁移 | pending-ready 表 |
| 运行任务删除后等待 idle 回收 | termination 表 | 空 |

队列有限等待时，`vTaskPlaceOnEventList` 插入有序事件表，再调用 `prvAddCurrentTaskToDelayedList` 移除 Ready 成员、建立截止时刻。队列无限等待在 `INCLUDE_vTaskSuspend=1` 且允许无限阻塞时使用 suspended 表，逻辑状态仍是 Blocked。显式挂起则撤销事件等待。不能只凭表名给状态下结论。[建立等待](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L5243-L5269)、[延时与无限等待](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L8415-L8531)、[显式挂起](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L3121-L3172)

`vTaskDelay` 传给内部函数的“可无限阻塞”参数为 false；不要把它的 `portMAX_DELAY` 与队列无限等待直接等同。`vTaskDelay(0)` 只请求重新调度，不建立正数延时。`vTaskResume` 也不能代替给队列发数据来唤醒无限队列等待者。[延时入口](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L2433-L2480)、[挂起判定与恢复](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L3280-L3408)

### 4. 事件到来与 tick 到期

普通队列事件唤醒走 `xTaskRemoveFromEventList`：取等待表头的任务，移除事件节点；调度器未挂起时再移除状态节点并加入 Ready。队列等待表按反向优先级值升序排列，表头通常对应最高优先级等待者。[有序事件解阻塞](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L5341-L5411)

tick 主线走 `xTaskIncrementTick`：增加计数，必要时交换延时表；达到下一解阻塞时刻时，循环处理所有已到期的表头。每个任务先移除状态节点，若仍有事件成员关系则移除事件节点，再加入 Ready。由此，一次有限等待不会因旧事件记录与旧截止记录各自触发而重复入 Ready。[tick 到期处理](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L4670-L4805)

这说明列表怎样避免保留两份有效等待，不说明“事件先唤醒就一定接收成功”。被唤醒后，任务还要真正获得 CPU，再重新检查对象。应用另外开启 `xTaskAbortDelay` 等功能时，也要检查额外的解除等待原因，不能把所有路径都归因于事件或自然超时。

### 5. 两张延时表处理回绕

设当前 tick 为 T、等待 d，唤醒值为无符号加法后的 `T+d`。唤醒值未越过本轮回绕时进入当前延时表；发生回绕时进入 overflow 表。tick 从最大值变成 0 时交换两张表，并重新确定下一解阻塞时刻。[表交换](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L246-L260)、[入表判定](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L8456-L8489)

原创算例：采用本章 32 位 tick，记最大值 M。当前为 M−3，A 延时 2 tick，唤醒值 M−1，进入当前表；B 延时 6 tick，唤醒值 2，进入 overflow 表。A 先到期，随后回绕交换，B 到 tick 2 才到期。只按裸数值把 2 排在 M−1 前面，会把两轮时间混在一起。

回绕时长由位宽和 tick 频率共同决定。例如 32 位、1 kHz 才约为 49.7 天；本章没有假定所有项目都用 1 kHz。`TimeOut_t` 还保存进入时刻与溢出计数，用于重复检查等待预算。[超时状态与预算](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L5506-L5600)

### 6. pending-ready 的适用范围

调度器挂起期间，合规 ISR 仍能运行。普通队列的有序事件唤醒不能直接按通常 ISR 路径改状态列表，所以先把事件节点放到 `xPendingReadyList`，状态节点暂留原表。最外层 `xTaskResumeAll` 恢复时，先清 pending-ready 的事件成员关系，移除旧状态节点并加入 Ready，再补处理挂起期间累计的 tick，最后按需要请求切换。[暂存与恢复](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L3956-L4103)

在这个暂存窗口，`eTaskGetState` 识别 pending-ready 并报告 Ready；不能把尚未移除的延时成员关系单独读成仍在等待。该查询也只是一个观察结果，不能替代应用寿命同步协议。

这一过程限定于上述有序事件路径。V11.1.0 事件组使用 unordered-event 函数，在其要求的调度器挂起条件下直接移除状态节点、加入 Ready，再延后切换；不能写成“所有解阻塞都先进入 pending-ready”。详见 [事件组条件](kernel-event-groups.md)。[unordered-event 入口](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L5434-L5503)

### 7. 删除不等于应用资源自动关闭

本章单核中，正在运行的任务删除自己时，TCB 的状态节点进入 termination 表，切换出去后由 idle 检查并回收相应内核存储。删除另一个未运行任务可走直接回收路径。静态存储不应由内核当动态堆块释放。[删除](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L2191-L2338)、[idle 清理](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L6043-L6072)、[存储来源分支](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L6401-L6453)

任务申请的应用缓冲、持有的锁、DMA、ISR 和定时器入口，都需要自己的关闭与归还协议。删除旧任务句柄后不能继续查询或重复删除；没有一个状态查询能替代停止所有引用者的证明。具体协议复习 [IC02](../120-interviews/ic02.md)，内存预算复习 [RT08](rt08.md)。

## 例子与推演

原创时序：R 优先级 4，在 tick 100 等空队列 Q，最多等 5 tick；W 优先级 2。

| 时刻或步骤 | R 的状态节点 | R 的事件节点 | 解释 |
| --- | --- | --- | --- |
| R 建立等待 | 延时表，值 105 | Q 接收等待表 | 两种唤醒条件对应两个节点 |
| ISR 在 tick 103 成功发入一项，调度正常 | Ready[4] | 空 | 解除等待，可能请求切换 |
| R 真正再次执行接收 | Ready[4]，R 为当前任务 | 空 | 根据 Q 的当前状态决定返回或再等 |

另画两个独立分支，不与上表混成一次执行：

- 超时先到：tick 105 移除 R 的两个等待关系，使 R Ready；之后到达的数据仍归 Q，接收返回如何判定查 [队列等待](kernel-queue-waits.md)。
- 挂起期间事件到达：R 的事件节点先在 pending-ready，状态节点暂留延时表；恢复时迁入 Ready，然后补 tick。要记录内核实际处理顺序，不能仅凭“同一个 tick 数字”给事件规定统一胜负。

## 常见误区与失败边界

- 一个 TCB 有两个节点，不能让其中一个节点同时进两张表；“任务被两张表引用”与“同一节点重复挂入”不同。
- 高优先级阻塞任务不消耗正常运行机会；但持锁任务阻塞可能扩大等待，继承也不会替它完成 I/O。
- tick 到期是解除等待资格，不是精确开始运行或完成工作的时刻。
- 长时间挂起调度器可能累计很多 tick 与待就绪任务，恢复工作随之增加，不能声称恢复总是固定开销。
- 饥饿使 idle 无法及时清理自删任务的内核存储；应用资源泄漏则需检查自己的归还路径。

## 面试追问

“怎么从内存判断任务状态？”答：先固定版本与配置，检查当前任务指针、状态容器、事件容器及通知状态；pending-ready 与无限阻塞尤其不能只看状态表名。用合法 API 做观察，别在应用里直接改内核结构。

“超时返回能证明数据没来过吗？”答：不能。数据可能在不同窗口到达、被其他接收者取走，或在失败后到达。最终业务结论需要返回值、序列号及应用协议，不能从一次解阻塞原因推断全局事件历史。

## 动手练习与验收

练习尚未执行。先建原创状态模型，再安排固定内核与目标 port 的验证：

1. 有限等待：列出事件先处理、截止先处理、同 tick 不同处理顺序；每次迁移检查两个节点的容器。
2. 回绕：用 8 位教学时钟模拟上面的两轮分类，另对照 32 位实际类型；8 位模型不是 ARM_CM4F 的 tick 配置。
3. 挂起：设置嵌套挂起，最外层恢复前不处理完 pending-ready；恢复后没有旧等待成员残留。
4. 无限等待与显式挂起：分别画表，解释为什么不能用相同唤醒操作。
5. 删除：分别追自删和删除未运行任务，检查 idle 进度、静态存储归属与应用资源退出。

验收：不重复入 Ready、不访问已回收句柄，每次预算减少与截止计算可解释。host 模型只验证其自身状态规则；需要真正运行内核/模拟器才能报告内核验证，硬件停止与实时延迟还需板级证据。

## 先修与复习入口

- [RT01 · 状态与调度](rt01.md)、[RT02 · 周期与延时](rt02.md)、[RT07 · 保护范围](rt07.md)
- [RT08 · 内存预算](rt08.md)、[RT10 · 创建与参数寿命](rt10.md)、[AR07 · 上下文切换](../30-architecture/ar07.md)
- [源码导读](source-reading.md)、[内核链表](kernel-lists.md)、[队列等待](kernel-queue-waits.md)、[事件组条件](kernel-event-groups.md)

## 来源与适用范围

- [固定提交 tasks.c](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c)、[task.h](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/include/task.h)：状态、创建、延时与删除的实现及 API 合同。
- [固定提交 ARM_CM4F port.c](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/portable/GCC/ARM_CM4F/port.c)：首次运行与异常切换，仅对所选 port 解释。
- [对应 MIT 许可](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/LICENSE.md)：本文不复制内核代码；以后分发源码或重要部分应保留对应版权与许可声明。

正文、时序和练习为原创教学内容。来源用于核对机制，不表示源码维护者为本讲义背书；SMP、其他 port、tickless 与可选解除阻塞路径需要另行核对。
