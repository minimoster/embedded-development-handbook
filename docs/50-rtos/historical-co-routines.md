# 历史选读 · FreeRTOS V9.0.0 co-routines 如何保存状态？

学习优先级：P2。用于理解旧工程与源码设计取舍，不增加 RT01–RT10 核心题数量。先读 [RT01 任务状态与调度](rt01.md)，再按 [源码阅读方法](source-reading.md) 固定版本。

本页限定官方 FreeRTOS V9.0.0、单核、历史 co-routine 实现。普通任务的调度配置、语言层面的 coroutine，以及手写事件状态机各有自己的语义，不能仅凭“协作式”三个字混用结论。

## 直接回答

FreeRTOS 历史 co-routines 通过控制块中的恢复位置和宏展开来续接执行，不为每个 co-routine 保存独立任务栈。它们相互之间按优先级协作调度；应用必须反复调用 `vCoRoutineSchedule()` 才能选择并运行它们。若调用者是 idle hook，优先级高于 idle task 的普通任务持续 Ready，就可能让 co-routines 长期得不到运行机会。[官方调度说明](https://www.freertos.org/Documentation/02-Kernel/02-Kernel-features/01-Tasks-and-co-routines/11-Scheduling-co-routines)

新工程通常先考虑普通任务，或边界清晰的非阻塞状态机。官方现行概览将 co-routines 定位于 RAM 极紧张的小设备，并说明如今很少使用、暂无移除及进一步开发计划；本页据此将它们列为历史选读，不把源码中仍有文件等同于新项目推荐。[官方概览](https://www.freertos.org/Documentation/02-Kernel/02-Kernel-features/01-Tasks-and-co-routines/00-Tasks-and-co-routines)

## 原理拆解

### 先区分三种执行方式

| 比较项 | 普通 FreeRTOS 任务 | V9.0.0 co-routine | 手写非阻塞状态机 |
| --- | --- | --- | --- |
| 跨等待保存什么 | 每个任务有自己的栈和执行上下文 | 控制块保存恢复位置；业务数据另行持久保存 | 应用显式保存阶段、数据与截止时间 |
| 谁安排下一次执行 | 任务调度器 | 应用反复调用 co-routine 调度入口 | 主循环、事件循环或承载任务 |
| 何处可以暂停 | 合适的任务 API；调用链由任务栈支持 | 受宏结构限制的 co-routine 函数体 | 每次处理函数主动返回，由应用设计入口 |
| 优先级作用范围 | 任务之间 | co-routines 之间；不能越过承载任务的调度条件 | 取决于应用自己实现的策略 |
| 主要维护成本 | 栈预算、同步与调度分析 | 恢复点限制、实例数据隔离、调度调用频率 | 状态转换、事件容量、超时及公平性都由应用负责 |

表中的状态机是教学比较模型，并非一个 FreeRTOS API。关闭普通任务抢占，即 `configUSE_PREEMPTION=0`，仍保留任务自己的栈和任务 API，不会把任务变成 co-routine。参见 [任务生命周期](kernel-task-lifecycle.md)。普通任务与 co-routines 的栈及 API 差异可在 [官方概览](https://www.freertos.org/Documentation/02-Kernel/02-Kernel-features/01-Tasks-and-co-routines/00-Tasks-and-co-routines)核对。

### 控制块保存位置，不保存任意调用栈

V9.0.0 的 `CRCB_t` 包含函数入口、两个链表节点、优先级、实例索引与 `uxState`。创建路径分配这个控制块，没有为它分配独立任务栈。宏在让出执行时记录恢复位置并从函数返回；下一次调度再次调用同一函数，由入口宏跳到对应恢复点。[V9.0.0 croutine.c](https://github.com/FreeRTOS/FreeRTOS/blob/V9.0.0/FreeRTOS/Source/croutine.c)、[croutine.h](https://github.com/FreeRTOS/FreeRTOS/blob/V9.0.0/FreeRTOS/Source/include/croutine.h)

因此，跨等待仍要使用的自动局部变量不能依赖原栈帧保存。需要持久数据时，可使用静态存储或按实例索引访问的持久结构。但一个函数内的单个 `static` 变量由所有调用该函数的实例共享，不会因创建两个 co-routines 就自动变成两份。索引也不会自动替应用分配数据。[官方使用限制](https://www.freertos.org/Documentation/02-Kernel/02-Kernel-features/01-Tasks-and-co-routines/12-Limitations-and-restrictions)

这里的“共享栈”仍有实际执行栈：co-routine 正在运行时使用调用者的执行环境。普通任务抢占这个调用者时，可以保存调用者的上下文；co-routine 自己阻塞、返回后，不会留下可在任意深层函数中继续的专属调用栈。

### 一个实例有两个节点，链表成员关系才是状态证据

阅读前先建立下面的账本；链表操作细节见 [内核链表](kernel-lists.md)。

| 节点 | 可能的容器 | 排序或用途 |
| --- | --- | --- |
| `xGenericListItem` | 某个优先级的 ready list，或一条 delayed list | ready 用于选择运行者；delayed 按唤醒 tick 排序 |
| `xEventListItem` | 某个队列的事件等待链表，或 pending-ready list，或无容器 | 等待链表按 co-routine 优先级组织；pending-ready 暂存待整理者 |

`uxState` 是恢复位置，不能单独回答“它现在 Ready 还是 Blocked”。一个等待队列并带超时的实例可以同时在 delayed list 和 event list 中，因为使用的是两个不同节点。同一个节点不能同时挂在两条链表。[croutine.c](https://github.com/FreeRTOS/FreeRTOS/blob/V9.0.0/FreeRTOS/Source/croutine.c)、[list.c](https://github.com/FreeRTOS/FreeRTOS/blob/V9.0.0/FreeRTOS/Source/list.c)

### 四条主要路径

1. 创建：初始化两个节点与恢复位置，将通用节点放入对应优先级的 ready list。检查 `xCoRoutineCreate()` 的返回值，不能假定内存分配成功。
2. 正 tick 的纯延时：将通用节点从 ready list 移到 delayed list；事件节点不挂到事件等待链表。宏保存恢复点并返回。到期整理后，通用节点回到 ready list。
3. 队列等待：队列条件暂时不满足且等待 tick 大于零时，通用节点进入 delayed list，事件节点进入该队列的发送或接收等待链表；宏随后返回。恢复时，队列宏会再做一次零等待尝试，最终结果仍需检查。
4. 事件解除等待：取事件等待链表中的首个实例，把它的事件节点移入 pending-ready；通用节点暂时还在 delayed list。调度入口整理 pending-ready 时，摘下事件节点和延时节点，再把通用节点放回 ready list。

第 4 条不限于 ISR：V9.0.0 的 co-routine 队列发送、接收路径也复用 pending-ready 机制。不要把 pending-ready 理解为另一条供调度器直接选择执行的 ready list。[queue.c 的 co-routine 路径](https://github.com/FreeRTOS/FreeRTOS/blob/V9.0.0/FreeRTOS/Source/queue.c)

超时路径会摘下通用节点，并在事件节点仍属于某个容器时把它一并摘下，再加入 ready list。事件与超时非常接近时，应逐步检查临界区和两个节点当时的容器；不能靠“事件总优先”或“超时总优先”推测结果。队列等待的共同思路参见 [队列等待链路](kernel-queue-waits.md)。

### 调度入口和 tick 分工

一次 `vCoRoutineSchedule()` 先整理 pending-ready，再检查经过的 tick 与延时到期，最后选择最高非空优先级 ready list 的下一个实例，并调用一次其函数。若没有 ready 实例，就返回。它不是自动安装在 tick 中断中的独立任务调度器。[V9.0.0 调度入口](https://github.com/FreeRTOS/FreeRTOS/blob/V9.0.0/FreeRTOS/Source/croutine.c)

同优先级通过链表游标轮流被选中，但某次函数执行多长时间，取决于它何时返回。一个长循环不让出，会挡住其他 co-routines；高优先级实例持续 Ready，也会压住低优先级实例。提高 co-routine 优先级无法修复 idle hook 得不到 CPU 的问题。

V9.0.0 用 `xTaskGetTickCount()` 与上次已处理 tick 的差值补查时间，并逐 tick 推进 co-routine 自己的计数。因此，tick 已经过了，仍要等调度入口运行才会整理到期实例；一次调用前漏过很多 tick，还可能带来较长的补查工作。实际执行时刻包含调度调用间隔和前序执行时间，不能把延时参数当作精确执行时刻。

## 例子与推演

### 无事件路径的 tick 回绕

为了手算，假设 tick 是 8 位无符号数，取值 0 到 255；这只是缩小位宽的教学模型，不能据此描述实际端口的 `TickType_t`。co-routine 计数为 250，A 请求延时 10 tick，记录的唤醒值为 4。

| 处理位置 | A 所在链表 | 原因 |
| --- | --- | --- |
| 插入后 | overflow delayed list | 唤醒值 4 小于当前值 250，属于下一轮计数 |
| 251 到 255 | 仍在 overflow delayed list | 尚未经过回绕 |
| 计数变成 0 | 原 overflow list 成为当前 delayed list | 两条延时链表交换角色 |
| 1 到 3 | 当前 delayed list | 当前计数尚小于唤醒值 |
| 4 | ready list | 超时整理摘下延时节点 |
| 后续选择 A 时 | 函数从恢复点继续 | Ready 到实际执行仍受调度约束 |

两条 delayed list 把“本轮到期”和“下一轮到期”分开，不靠直接比较所有回绕后的 tick 值解决问题。[V9.0.0 延时管理](https://github.com/FreeRTOS/FreeRTOS/blob/V9.0.0/FreeRTOS/Source/croutine.c)

差值补查也有边界：若两次调用间隔跨过完整计数周期，单靠模运算差值无法知道到底漏过多少轮。例如 8 位模型过了 256 tick，读数仍可能相同。这是由该实现的差值计算推得的限制；应结合实际 tick 位宽与调度调用频率评估，不能推成所有 FreeRTOS 任务延时的限制。

### 队列事件路径的两个节点

设 A 在 tick 100 等待空队列，最长等 8 tick；ISR 在 103 成功写入数据。假设这一段没有其他消费者，也没有在超时整理临界区附近交错的事件。

| 时刻或步骤 | A 的通用节点 | A 的事件节点 | 此时能得出的结论 |
| --- | --- | --- | --- |
| 100，等待登记完毕 | delayed，唤醒值 108 | 队列接收等待链表 | A 已让出，等待数据或超时 |
| 103，ISR 处理事件后 | 仍在 delayed | pending-ready | 等待已获唤醒安排，A 尚未执行 |
| 下次调度整理后 | ready | 无容器 | 超时登记已清理，A 可被选择 |
| A 被调用并恢复 | ready | 无容器 | 队列宏重试接收；须检查最终结果 |

若没有数据到来，超时整理会清理两条等待登记，恢复后的零等待接收失败。若有另一消费者先拿走了数据，事件唤醒也不保证 A 最终接收成功。把“已唤醒”和“已取得数据”分开检查。[V9.0.0 队列宏](https://github.com/FreeRTOS/FreeRTOS/blob/V9.0.0/FreeRTOS/Source/include/croutine.h)、[队列实现](https://github.com/FreeRTOS/FreeRTOS/blob/V9.0.0/FreeRTOS/Source/queue.c)

### 用手写状态机说明持久数据的归属

下面是原创伪代码，展示两个采样通道各自保存校准进度。它不是 FreeRTOS 实现，不使用 co-routine 宏，也不是已编译的 C 程序。

```text
持久实例数组 channel[2]：每项保存阶段、校准累计值、截止时间
初始阶段均为“开始下一轮”

step(实例编号, 当前时间, 本轮事件)：
    若阶段是“开始下一轮”：
        清零这个实例的累计值，设置本轮截止时间
        阶段改为“等待样本”
        返回

    若阶段是“等待样本”：
        若本轮有匹配的样本：
            累加到这个实例的累计值
            若样本已足够，阶段改为“准备输出”
        否则若已到截止时间：
            记录这个实例的超时，阶段改为“准备输出”
        返回

    若阶段是“准备输出”：
        非阻塞尝试写入输出缓冲
        若成功，阶段改为“开始下一轮”
        返回

主循环：
    对两个实例各调用一次 step
    再处理其他事件
```

这里把恢复位置写成显式阶段，把跨次调用数据放在每个实例的结构中。伪代码自行选择同轮已有匹配样本时先处理样本；截止判断还需按实际计数位宽与时间跨度处理回绕。若改成历史 co-routine 宏，虽然顺序书写更方便，实例隔离、调用频率、队列容量和失败处理仍要设计；仅把局部变量加上 `static` 并没有完成这些工作。

## 历史配置与 API 阅读清单

下列名称以 V9.0.0 为准；这是阅读入口，不是建议新项目开启的模板。

- `configUSE_CO_ROUTINES=1`：启用相关实现，并把 `croutine.c` 纳入构建。V9.0.0 的 `FreeRTOS.h` 未定义该配置时默认置零。
- `configMAX_CO_ROUTINE_PRIORITIES`：启用时需定义，有效优先级为零到该值减一；数值较大优先级较高。不要拿它代替任务的 `configMAX_PRIORITIES`。
- `xCoRoutineCreate(function, priority, index)`：创建实例；这个版本通过分配器取得控制块。索引用来区分使用同一函数的多个实例。
- `vCoRoutineSchedule()`：由应用反复调用。采用 idle hook 时，还需正确配置 `configUSE_IDLE_HOOK`，并让 hook 中每次工作有界返回。
- `crSTART`、`crEND`：包住协程函数体；`crDELAY` 提供延时恢复点。V9.0.0 中零 tick 的 `crDELAY` 仍会保存恢复点并返回，但不进入延时链表。
- `crQUEUE_SEND`、`crQUEUE_RECEIVE`：使用带恢复逻辑的 co-routine 宏；通过结果指针检查最终成败。内部的 `errQUEUE_BLOCKED`、`errQUEUE_YIELD` 用于宏续接，不应按普通任务队列返回值随意套用。
- `crQUEUE_SEND_FROM_ISR`、`crQUEUE_RECEIVE_FROM_ISR`：ISR 与 co-routine 队列交互的历史接口。它们不在 ISR 中等待；参数和返回语义与普通任务的同类 API 不完全相同。

核对入口：[V9.0.0 FreeRTOS.h](https://github.com/FreeRTOS/FreeRTOS/blob/V9.0.0/FreeRTOS/Source/include/FreeRTOS.h)、[croutine.h](https://github.com/FreeRTOS/FreeRTOS/blob/V9.0.0/FreeRTOS/Source/include/croutine.h)、[croutine.c](https://github.com/FreeRTOS/FreeRTOS/blob/V9.0.0/FreeRTOS/Source/croutine.c)。

## 常见误区与失败边界

- “没有独立栈，所以不会发生栈问题”：正在执行的函数和普通辅助函数仍消耗调用者的栈；过深调用仍需评估。
- “任何辅助函数中都可以阻塞”：可能暂停的 co-routine 宏须放在 co-routine 函数本体。宏恢复不了任意辅助函数的栈帧。
- “放在自己的 switch 分支里也一样”：V9.0.0 默认宏通过 switch/case 实现恢复；官方限制禁止在应用 switch 内放入可能阻塞的调用。
- “把两个恢复宏挤在一行只是排版”：该版本恢复标签由源码行号生成；相同源码行放多个恢复点可能产生标签冲突。检查预处理结果时，不应把宏当普通函数调用。
- “同优先级轮流就是固定 CPU 时间片”：轮流选择不限制每次运行时长；须让每段工作有界并及时返回。
- “ISR 唤醒后马上运行”：先进入 pending-ready，仍需后续调度调用。不能在 ISR 随意调用调度入口或操作 ready lists。
- “一个队列可直接混用任务与 co-routine API”：官方明确说明两类 API 不能借同一队列或信号量直接互传数据。V9.0.0 中队列等待链表字段名含 `Tasks`，也不能据名字推断它可以混用等待者。
- “`portMAX_DELAY` 一定意味着永久等待”：本页只确认该 co-routine 路径计算有限 tick 唤醒值，不能套用普通任务在特定配置下的无限等待语义。

宏结构和 API 使用限制见 [官方限制页](https://www.freertos.org/Documentation/02-Kernel/02-Kernel-features/01-Tasks-and-co-routines/12-Limitations-and-restrictions)与 [V9.0.0 croutine.h](https://github.com/FreeRTOS/FreeRTOS/blob/V9.0.0/FreeRTOS/Source/include/croutine.h)；队列混用限制见 [官方任务与协程概览](https://www.freertos.org/Documentation/02-Kernel/02-Kernel-features/01-Tasks-and-co-routines/00-Tasks-and-co-routines)。

## 面试追问

**把 `configUSE_PREEMPTION` 置零能节约每个任务的栈吗？**

不能据此减少独立任务栈数量。它改变任务之间的调度方式；历史 co-routines 通过不同的执行与恢复模型减少独立栈需求，两者不能互相替代。

**一个队列宏成功发送后为什么还可能让出？**

成功操作可能解除另一个 co-routine 的等待。V9.0.0 在被唤醒者优先级大于或等于当前者时，可返回内部的 `errQUEUE_YIELD`；宏保存另一个恢复点，下一次恢复时把业务结果置为成功。不能把这条内部让出路径解释为发送失败，更不能重发一次制造重复数据。[croutine.c 与优先级比较](https://github.com/FreeRTOS/FreeRTOS/blob/V9.0.0/FreeRTOS/Source/croutine.c)、[队列宏](https://github.com/FreeRTOS/FreeRTOS/blob/V9.0.0/FreeRTOS/Source/include/croutine.h)

**为什么 event list 和 delayed list 都有它，却没有重复运行？**

两种登记分别用两个节点。事件、超时处理路径会整理剩余登记，最终由通用节点进入 ready list。要证明某次交错没有重复加入，需画出具体节点容器与临界区，不能仅说“内核会处理”。

**什么时候选手写状态机？**

流程较短、阶段清楚、已有事件循环且能明确限定每次处理工作量时，可以评估显式状态机。若流程包含复杂调用链、多个独立等待与同步关系，普通任务常更易表达和维护。这是设计判断，仍需按实际工程验证内存预算、响应需求与可维护性。

## 动手练习与验收

以下均是待执行练习；本页未编译示例、未运行目标板或模拟器测试。前面的时序表是纸面推演，不是测试日志。

1. 状态账本：给 A、B 各画通用节点与事件节点，依次安排创建、纯延时、队列等待、ISR 写入、超时。验收：每一步写清容器、恢复点和最终队列结果；不能把 pending-ready 当已执行。
2. 回绕：用 8 位纸面模型，在 250 延时 10；再加入一个唤醒值为 253 的实例。验收：解释先处理哪条链表、何时交换角色，以及完整周期遗漏为何无法靠一次差值恢复。
3. 实例隔离：让两个实例复用同一业务函数，先分析单个静态累计值的共享问题，再改用按索引分开的持久数据。验收：任一实例等待时，另一实例更新不覆盖它的业务状态。
4. 调度饥饿：假设调度入口只在 idle hook 调用，加入一个优先级高于 idle task、始终 Ready 的普通任务。验收：能解释提高 co-routine 优先级为什么无效，并提出让普通任务合理阻塞、限制单次工作量的方案。
5. 边界交错：让事件恰好发生在 pending-ready 已检查、超时节点准备清理之间。只读官方源码，按实际关中断范围推演两个节点变化。验收：说明事件节点可能属于事件链表或 pending-ready，以及恢复后为何仍须检查队列结果。

如要进一步做实验，先固定官方版本、端口、tick 位宽、配置与工具链。记录创建返回值、调度入口时间、实例编号、恢复前后队列结果；不要把示意日志写成测得的延迟保证。

## 来源与适用范围

- [FreeRTOS 官方 V9.0.0 标签](https://github.com/FreeRTOS/FreeRTOS/tree/V9.0.0)：本页历史实现基线；标签解析至提交 `165c24cba94e3d3f386c3b77d3bb0f4a524d8511`。
- [croutine.c](https://github.com/FreeRTOS/FreeRTOS/blob/V9.0.0/FreeRTOS/Source/croutine.c)：创建、调度、延时、pending-ready 与事件解除等待路径。
- [croutine.h](https://github.com/FreeRTOS/FreeRTOS/blob/V9.0.0/FreeRTOS/Source/include/croutine.h)：控制块、恢复宏和历史队列 API。
- [queue.c](https://github.com/FreeRTOS/FreeRTOS/blob/V9.0.0/FreeRTOS/Source/queue.c)：仅据其中 `configUSE_CO_ROUTINES` 路径核对等待与 ISR 交互。
- [list.c](https://github.com/FreeRTOS/FreeRTOS/blob/V9.0.0/FreeRTOS/Source/list.c)与 [list.h](https://github.com/FreeRTOS/FreeRTOS/blob/V9.0.0/FreeRTOS/Source/include/list.h)：节点容器、排序插入和轮转游标。
- [FreeRTOS.h](https://github.com/FreeRTOS/FreeRTOS/blob/V9.0.0/FreeRTOS/Source/include/FreeRTOS.h)：历史配置默认值及检查。
- [官方现行概览](https://www.freertos.org/Documentation/02-Kernel/02-Kernel-features/01-Tasks-and-co-routines/00-Tasks-and-co-routines)、[调度说明](https://www.freertos.org/Documentation/02-Kernel/02-Kernel-features/01-Tasks-and-co-routines/11-Scheduling-co-routines)及 [限制说明](https://www.freertos.org/Documentation/02-Kernel/02-Kernel-features/01-Tasks-and-co-routines/12-Limitations-and-restrictions)：于 2026-10-10 核对定位与使用边界，不能用来替代项目实际版本的源码。

V9.0.0 所核对文件头部声明 GPLv2 加 FreeRTOS exception；对应 [该版本许可文件](https://github.com/FreeRTOS/FreeRTOS/blob/V9.0.0/FreeRTOS/License/license.txt)。不要用现代版本的许可名称覆盖旧版本。本文只作原理说明，不转载实现、原注释或官方示例。

正文、伪代码、时序推演与练习为本仓库原创教学内容。来源用于核对机制，不表示原作者为本仓库背书；没有将本页推演作为任何实际端口的测试结论。
