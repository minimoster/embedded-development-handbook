# 软件定时器与共享 daemon 的命令和寿命

学习优先级：P1，内核阅读补充。先能回答 [RT10](rt10.md)，再沿本章区分请求、到期、回调执行和资源释放。本章补充 RT01–RT10 的机制阅读，不改变基础题的学习顺序。

核对日：2026-10-10。源码基线为 **FreeRTOS-Kernel V11.1.0**，tag 对应提交 `dbf70559b27d39c1fdb68dfb9a32140b6a6777a0`。本章的解释、简化模型、场景和练习为原创；只做了公开源码与文档核读，**未编译或运行主机模型、内核实验和板级实验**。文中的时序数值是推演条件，不是测量结果。

## 直接回答

FreeRTOS 软件定时器把“将来需要处理的事”交给一个共享的 timer service task，也称 daemon task。定时器到期回调在这个任务的上下文执行。启动、复位、停止、改周期和删除通常先向内部命令队列发送请求，再由 daemon 修改定时器状态。

因此必须分清三个时刻：**命令被队列接收、daemon 完成操作、回调实际开始执行**。`xTimerStop()` 或 `xTimerDelete()` 返回 `pdPASS`，只保证相应命令成功入队；不能据此立即释放回调仍可能访问的对象。回调应有界且不阻塞，否则会拖住其他软件定时器和共享 daemon 的延后工作。[V11.1.0 API 契约](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/include/timers.h#L451-L671)

## 适用范围与配置前提

下面的调度和 ISR 讨论限定为单核、抢占式 FreeRTOS，普通 GCC `portable/GCC/ARM_CM4F` 移植，不覆盖 SMP 或 MPU 包装层。时间示例采用 32 位 tick、`configINITIAL_TICK_COUNT=0`；只有写明 1 kHz 的例子才按 1 tick = 1 ms 换算。不能把示例频率当成内核默认值。

读项目时先写下这些配置：

- `configUSE_TIMERS=1`，并把 `timers.c` 加入构建
- `configTIMER_TASK_PRIORITY`：daemon 的任务优先级，影响它获得 CPU 的机会
- `configTIMER_QUEUE_LENGTH`：内部队列能容纳的待处理命令数量
- `configTIMER_TASK_STACK_DEPTH`：daemon 的栈深度；本移植以 `StackType_t` 元素计，元素为 32 位，不能直接当字节数
- `configSUPPORT_DYNAMIC_ALLOCATION`、`configSUPPORT_STATIC_ALLOCATION`：定时器对象以及服务任务存储采用哪条分配路径
- `INCLUDE_xTimerPendFunctionCall=1`：使用延后函数调用，包括本章的确认标记方案时需要
- 若使用事件组的 ISR 延后操作，还要包括事件组模块，并核对 `configUSE_EVENT_GROUPS=1`

V11.1.0 在启用 timers 后要求定义 `configTIMER_TASK_PRIORITY`、`configTIMER_QUEUE_LENGTH` 和 `configTIMER_TASK_STACK_DEPTH`。`INCLUDE_xTimerPendFunctionCall` 默认关闭，它和 `configUSE_TIMERS` 的依赖也需要检查。启用静态分配时，daemon 的创建路径会取得静态任务内存；需要核对 `vApplicationGetTimerTaskMemory()` 的实际提供方式和缓冲寿命。不能因为某个 timer 使用静态创建，就跳过服务任务、命令队列及其初始化检查。[配置检查](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/include/FreeRTOS.h#L516-L534)；[延后开关](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/include/FreeRTOS.h#L283-L335)；[daemon 创建](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/timers.c#L237-L332)；[本移植的数据类型](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/portable/GCC/ARM_CM4F/portmacro.h#L50-L82)

## 原理拆解

### 1 一个对象 两类容器 一个执行者

一个定时器控制块保存周期、回调函数、名称指针、ID、状态位及内嵌链表项。它不自带一个独立任务，也没有一份每个定时器专用的执行栈。

内部有两类容器，职责不同：

1. **命令队列**保存等待 daemon 处理的请求，例如 start、stop、reset、change-period、delete。启用延后函数调用后，同一队列也装载函数和参数的请求。
2. **活动到期链表**保存已安排的定时器，按到期 tick 排序。当前链表与溢出链表一起处理 tick 回绕。这些活动链表由 daemon 操作。

到期回调从活动链表的处理路径直接调用；延后函数从命令队列的处理路径调用。V11.1.0 没有再给所有到期回调建立一个独立的“回调队列”。应用若把回调产生的工作投递到自己的工作队列，那是应用的另一层设计，应单独核对容量和寿命。[控制块、消息和活动链表](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/timers.c#L78-L157)

可以用以下原创概念模型理解职责，**这不是内核源码的替代实现，也省略了回绕、调度锁和队列解锁细节**：

```text
应用请求：检查句柄与上下文 → 形成命令 → 尝试交给内部队列

daemon 一轮工作：
    查看最近的到期安排
    若需要处理到期，就更新安排并调用短回调
    否则等待“有命令”或“到下一个检查时刻”
    逐项处理命令；延后函数请求也在这里执行
    回到下一轮，重新检查时间和安排
```

在 V11.1.0 中，`prvTimerTask()` 的主循环先处理到期或等待，再处理收到的命令。命令处理循环一直读取到队列为空，不能据此推导出每轮固定处理 N 个命令的公平性保证。持续的命令流、长回调和其他任务的抢占都要进入负载分析。[daemon 主循环与等待路径](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/timers.c#L748-L842)；[命令处理](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/timers.c#L938-L1084)

### 2 创建只建立对象

`xTimerCreate()` 动态申请定时器控制块，失败返回 `NULL`。`xTimerCreateStatic()` 使用应用提供的 `StaticTimer_t` 缓冲；缓冲必须有效且足够长寿命。新对象处于 dormant 状态，创建成功不会自动启动倒计时。创建期间也可能初始化服务所需的链表和队列；创建返回一个非空 timer 句柄，不等于整套服务任务已经成功开始运行。[创建与初始化路径](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/timers.c#L334-L446)

三个容易遗漏的寿命问题：

- `pcTimerName` 是用于调试的名称。该版本保存传入的指针，并不复制字符串。返回后失效的局部字符数组不能用作长期名称；字符串字面量或合适的长寿命存储更容易管理。
- `pvTimerID` 保存一个 `void *` 值。若它指向应用上下文，内核不会复制目标对象，也不会延长目标寿命。上下文必须活到最后一次合法读取；回调又向工作任务传出指针时，还要覆盖工作任务的读取期。
- 删除动态 timer 释放的是内核分配的控制块；不会自动释放名称、ID 指向的上下文或应用队列项中的缓冲。静态 timer 的缓冲仍属于应用，重用也要等旧操作已经静止。

`pvTimerGetTimerID()`、`vTimerSetTimerID()` 在此版本直接访问控制块，并不是排队命令。不能把所有名字带 Timer 的 API 都画成一次队列往返；临界区保护指针字段，也不能替应用证明指针目标还活着。[ID 访问](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/timers.c#L1191-L1228)；[删除分支](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/timers.c#L1051-L1075)

### 3 接收命令与完成动作

对经命令队列处理的 API，`xTicksToWait` 是**调用任务为队列空位等待的时间**。它不是倒计时周期，不是等待 daemon 完成操作的期限，也不是回调执行预算。

例如 `xTimerStart(timer, 5)` 中的 5：若队列有空位，立即尝试入队；若已满，调用任务可等待空位。返回 `pdPASS` 说明请求已交给队列。daemon 优先级较高时，可能在调用者返回前就处理了请求；daemon 较低时，处理可能还没开始。代码不能依赖这两种时序中的某一种作为通用完成保证。[任务命令发送](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/timers.c#L448-L497)；[start 返回值](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/include/timers.h#L487-L507)

调度器启动前可以创建、启动 timer，但 daemon 尚不能消费请求。此时 timer 命令发送不等待空位，启动请求会受队列容量限制。不要在启动前批量发送很多请求后忽略失败；预启动的 start/reset 也不能拿墙上经过的时间算到期，它们的语义要按调度器启动后的 tick 基准理解。`vTaskStartScheduler()` 在启用 timers 时会创建 daemon。[预启动契约](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/include/timers.h#L477-L492)；[调度器启动路径](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L3665-L3718)

### 4 启动 复位 改周期的时间基准

把时间参数逐项写清楚：

- `xTimerStart()`：dormant timer 进入 active；对已 active timer，效果等同 reset。
- `xTimerReset()`：重新安排到期。V11.1.0 的 start/reset 宏取调用路径上的 tick 时间戳，daemon 用该时间戳加当前周期计算计划到期值。因此队列拥堵不会统一变成“处理命令后再等一个完整周期”。若命令发送过程中为队列空位阻塞，这段等待也可能消耗周期。
- `xTimerChangePeriod()`：设置非零新周期并使 timer active。此版本 daemon 用**处理该命令时采样的当前 tick**加新周期安排下一次到期，与 start/reset 的时间戳基准不同。
- `xTimerStop()`：daemon 处理时取消 active 安排，保留对象供以后启动。
- `xTimerDelete()`：daemon 处理时移除安排并删除对象。应用在删除请求被接受后就应停止使用该句柄，不能继续查询它来“等待删除完成”。

这些行为要对照固定版本的宏与命令分支，不能由函数名猜时间基准。[start/reset 宏与契约](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/include/timers.h#L676-L796)；[daemon 的各类命令](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/timers.c#L995-L1083)

周期必须大于 0。把毫秒换成 tick 后仍要检查结果：短时间参数可能量化为 0；量化方向和整数溢出取决于实际转换宏与类型。tick 数是计划时间刻度，并不承诺从某次 C 函数调用的真实时刻起，精确经过同样长的墙上时间。

### 5 单次与自动重载

one-shot 到期处理后进入 dormant，控制块仍存在；以后可以重新 start/reset。auto-reload 则安排后续周期。V11.1.0 的正常重载沿“前一次计划到期值 + 周期”推进，不是每次沿“实际回调结束时刻 + 周期”推进。

这一安排有助于维持计划节拍，但不保证每个回调都准时。daemon 恢复运行时，多个计划周期可能已经过了；`prvReloadTimer()` 会处理积压，在同一轮中连续执行多次回调，再留下未来的到期安排。不要默认超载时内核自动只保留最新一次。

回调参数只有 timer 句柄，没有“本次计划到期时间”参数。该版本的积压重载路径和外层到期路径都可能调用回调，应用不能把每次回调的现场 `now` 当成采样发生时间，也不能仅靠回调调用顺序推断每个历史样本的时间标签。需要采样时间的业务应在真正采样处记录时间。[到期与积压重载](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/timers.c#L699-L746)；[回调签名](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/include/timers.h#L78-L90)

如果业务允许合并，可以由短回调只标记“需要更新”，让工作任务处理最新状态；如果每次事件都必须保留，则要有可计数的投递、容量上界和溢出策略。若回调自身耗时接近或超过周期，补处理反而可能制造更多迟到，需重新设计周期或工作分配。周期任务的类似问题可复习 [RT02](rt02.md)。

### 6 回绕与插入函数的 Boolean 语义

tick 是有限宽度无符号计数。跨过最大值后，新到期数值可能很小，不能只用 `expiry < now` 判定已经迟到。活动链表分成当前时间段和溢出后的时间段；daemon 采样到回绕时，处理旧时间段残留的到期项，再交换两条链表的角色。

阅读 `prvInsertTimerInActiveList()` 时，最重要的是看返回变量的含义：

- **`pdTRUE` 表示这个到期应立即处理**，该路径没有把它作为未来到期项插进链表
- **`pdFALSE` 表示本次没有要求立即处理**，正常路径已放入当前链表或溢出链表

这里的 Boolean 不是“插入成功/失败”。它是交给调用者的处理决策。若把 `pdTRUE` 翻译成成功，会把 start/reset 的迟到回调分支读反。[插入决策](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/timers.c#L894-L936)；[时间采样](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/timers.c#L871-L891)；[链表切换](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/timers.c#L1086-L1109)

下面用 **8 位纸面计数器**缩小数值，便于手算；这是原创模型，不是声称本章 ARM_CM4F 配置支持 8 位 tick。运算按模 256 进行，观察间隔不跨多个完整计数周期。

| 请求时间 c | 周期 P | 到期 E | 处理时刻 n | 推演结果 |
| --- | ---: | ---: | ---: | --- |
| 100 | 10 | 110 | 115 | 已经过 15 tick，立即处理，返回 pdTRUE |
| 250 | 10 | 4 | 252 | 只经过 2 tick，到期在下一时间段，插入溢出链表，返回 pdFALSE |
| 250 | 10 | 4 | 2 | 已回绕但距离到期还有 2 tick，插入当前链表，返回 pdFALSE |
| 250 | 3 | 253 | 2 | 到期发生在回绕前，命令处理时已经迟到，立即处理，返回 pdTRUE |

读源码时同时记录请求时间、当前时间、计划到期值和所属链表。简单写一句“无符号减法天然解决所有回绕”不足以证明任意长暂停都正确；有限位宽不能表达无限长的欠账历史。

## 例子与推演

### 场景一 start 成功 回调仍迟到

条件：周期 10 tick，daemon 优先级低于一个持续 Ready 的计算任务。没有 stop/reset/change-period，也没有回绕；回调本身耗时可忽略。

```text
tick 100：控制任务调用 start，命令携带时间戳 100，成功入队
tick 100 到 135：高优先级计算任务持续占用任务执行时间
tick 136：daemon 才处理 start；计划到期值为 110，已经迟到
```

one-shot 将在这次迟到处理时调用一次回调，并进入 dormant。auto-reload 会发现 110、120、130 三个计划点已经过去，在这个采样时刻的积压处理路径产生三次回调调用，并留下 140 的未来安排。这里列的是欠下的计划点和调用数量，**不把内核的各次调用顺序标成按历史计划点逐项有序播放**。

该推演要分开记录：请求入队成功、计划点 110、实际处理约 136。`pdPASS` 没有给出回调最坏延迟。更高 daemon 优先级可能减少这类等待，但慢回调、ISR、临界区和其他更高优先级负载仍然需要预算。

### 场景二 回调等自己的消费者腾出队列

条件：内部命令队列已满。回调在 daemon 中调用 `xTimerStop(self, portMAX_DELAY)`，并按允许无限等待的项目配置等待队列空位。

```text
daemon 正在运行回调
    回调要向 daemon 的命令队列写 stop
    队列满，daemon 因等待空位而阻塞
    只有 daemon 能消费这个队列
    daemon 又要等回调返回后才能继续消费
```

形成自等待。有限非零等待虽然最终可能超时，也会把整个服务拖住。在 daemon 回调中调用带等待参数的 API，应使用 0，检查失败，并把需要重试的工作交给其他受控任务。

即使 `xTimerStop(self, 0)` 成功，也只是入队。积压重载路径可能在 daemon 回头处理 stop 前再次调用这个 timer 的回调，所以“回调第 N 次提交 stop”不能单独保证业务恰好执行 N 次。若必须严格限次，还需在有界回调里维护明确的业务门控和同步规则。[头文件中的回调阻塞边界](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/include/timers.h#L157-L185)

### 场景三 一个慢回调影响三个对象

A 每 20 tick 到期，B 每 25 tick 到期；一个 ISR 同时请求事件组置位。若 A 回调同步等串口发完 40 ms，daemon 就暂时不能执行 B 回调，也不能执行队列中该事件组的延后置位。

问题不只来自 `vTaskDelay()`。同步驱动等待、忙轮询、过长日志格式化和复杂计算即使没有进入 FreeRTOS Blocked 状态，也会占用共享执行者。设计时需给每个回调和延后函数一个可验证的最坏执行预算，审查其全部调用链。

## 回调工作交接

回调适合做短的状态更新、读取稳定上下文、向工作任务投递轻量请求。回调是任务上下文，通常使用任务版投递 API；它并不因为名称叫 callback 就变成 ISR。

下面是原创业务伪代码，省略了创建、同步和错误处理的具体 API，**不能作为可直接烧录的完整工程**：

```text
timer_callback(handle):
    context = 获取该 timer 的 ID
    若当前会话已进入停止阶段：立即返回
    request = 复制本轮需要的少量信息和会话代号
    以零等待投递到应用工作队列
    若投递失败：累积失败统计，按既定丢弃或合并策略处理
    返回

worker_task:
    阻塞等待应用工作队列
    核对会话代号和状态
    完成允许耗时的 I/O 或计算
    归还自己持有的缓冲
```

状态检查本身需要适合项目的同步，不能用一个裸共享布尔量替代协议。若 request 带的是指针，复制 request 不会复制指针目标；仍需拥有者、归还事件及停止时的在途统计。任务通知可以实现某些轻量交接，但选择位、计数或覆盖语义时要核对允许合并的业务边界。

## ISR 路径与共享 daemon

V11.1.0 提供 `xTimerStartFromISR()`、`xTimerResetFromISR()`、`xTimerStopFromISR()` 和 `xTimerChangePeriodFromISR()`。它们不能等待空位，队列满会失败；成功仍表示命令被接收。`pxHigherPriorityTaskWoken` 反映可能需要请求任务切换，不能当成 timer 操作已经完成的证明。[ISR 命令发送](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/timers.c#L499-L540)

`xTimerCreate()` 和 `xTimerCreateStatic()` 不属于 ISR 安全接口。此版本没有 `xTimerCreateFromISR()` 或 `xTimerDeleteFromISR()` 供应用套用；不要在 ISR 里创建或删除 timer。先在初始化或任务中创建，再让合规 ISR 使用明确的 FromISR 操作；删除交给有寿命协议的控制任务。

在本章 GCC ARM_CM4F 前提下，还必须检查实际 NVIC 优先级、有效优先级位与分组，满足 `configMAX_SYSCALL_INTERRUPT_PRIORITY` 的可调用边界。数值越小通常中断越紧迫，不能套任务优先级的数值方向。每次 ISR 进入时初始化 woken 为 `pdFALSE`，检查操作返回值，并按移植的 `portYIELD_FROM_ISR(woken)` 请求切换。[RT06](rt06.md) 给出基础核对步骤；[本移植的优先级断言](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/portable/GCC/ARM_CM4F/port.c#L853-L909)给出源码边界。

事件组还有一层容易漏掉的延后关系：此版本 `xEventGroupSetBitsFromISR()` 通过 `xTimerPendFunctionCallFromISR()` 把置位工作送给同一个 daemon。它返回成功时，目标位不一定已经改变，等待目标位的任务也不一定已经 Ready。`xEventGroupClearBitsFromISR()` 同样有延后路径。由此可以推导：timer 回调过长会影响事件组 ISR 请求的实际处理；命令队列不足也会影响这些请求。[事件组置位的 ISR 契约](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/include/event_groups.h#L543-L620)；[清位的延后路径](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/include/event_groups.h#L398-L461)；[延后函数调用的队列路径](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/timers.c#L1231-L1293)

## 停止与删除要有静止确认

考虑这个错误顺序：控制任务提交 `xTimerStop()`，返回 `pdPASS` 后马上 free ID 上下文，或者马上删除回调要投递的应用队列。此时 daemon 可能还在执行回调，也可能先处理一个已经到期的 timer，再处理 stop 请求。应用发生释放后访问，不能归咎于队列“不够线程安全”。

推荐沿 [IC02](../120-interviews/ic02.md) 的关闭协议组织：`RUNNING → STOPPING → QUIESCENT → RELEASED`。以下步骤是原创设计建议，**不是 FreeRTOS 自带的 join API 或设备关闭保证**：

1. 进入 STOPPING，关闭新 start/reset/change-period 和直接访问 timer 的入口。让所有相关任务与 ISR 按同步协议停止提交请求；停止硬件生产并确认在途路径不再提交旧句柄。
2. 保持 timer 的 ID 上下文、应用队列和相关事件组有效。由控制任务提交 stop 或 delete，检查返回值；失败意味着停止操作还没有交给 daemon，不能释放资源。
3. 取得 daemon 处理已提交工作的确认。确认必须来自可证明的协议，不能靠“延时几毫秒应该够了”或一次 active 查询。
4. 确认工作任务已经处理完或按业务协议丢弃旧请求，归还缓冲，不再访问共享对象。daemon 静止确认不会自动清空应用工作队列，也不会终止 DMA。
5. 只有所有使用路径已静止才进入 RELEASED。关闭超时要保留仍被使用的资源并报告失败；不能把超时解释成已经安全。

### 一个有条件的 daemon 确认标记

若启用了 `INCLUDE_xTimerPendFunctionCall=1`，可以设计一个短的延后确认函数：控制任务先成功提交 delete，再成功提交确认标记；daemon 处理标记时，用非阻塞通知给控制任务回执。该方案利用此版本命令按队列尾部入队并依次取出的顺序。

这个标记只在明确的条件下有意义：

- 所有旧 timer 请求、旧事件组 ISR 延后请求的生产者已经停止，并保证标记之后不会再入队
- delete 和标记按预期顺序均成功提交；若标记入队失败，保持资源，后续仅重试标记，不能重复使用已提交删除的旧句柄
- 标记参数和被通知的控制任务存活到处理完成；标记函数不再解引用已删除的 timer 或待释放的事件组
- 确认函数先完成对自身参数的最后一次访问，再发专用回执，此后不再访问待释放对象；回执不会与旧会话混淆
- 等待发生在控制任务中；daemon 不等待该控制任务，也不等待应用工作结束

满足这些条件时，标记的回执可证明同一队列中标记之前的工作已经处理，并证明此前同一 daemon 上的回调已经返回。删除 timer 后，再通过其他协议确认工作任务的剩余请求处理完，才允许释放应用上下文。若只提交 stop，标记也不能阻止其他未受控入口将来重新启动 timer。

类似地，关闭事件组时需要先停止所有相关 ISR 和任务入口，再确认已入队的延后置位/清位不再引用它，最后删除。把共享变量设成 `NULL` 不能撤回命令队列中已经复制的旧指针。这个顺序证明依赖固定版本和应用协议，不能泛化为“调用 pend 就自动安全”。[命令消费](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/timers.c#L938-L1084)；[队尾提交确认标记](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/timers.c#L1261-L1293)

## 实时性与容量怎么验收

测量至少保留五类信息：请求调用时刻、命令是否成功接收、计划到期值、回调实际开始/结束时刻、工作任务开始/结束时刻。到期值属于安排，实际开始属于执行；在处理好回绕和单位的前提下，二者之差才反映回调迟到。`xTimerGetExpiryTime()` 对非运行 timer 的返回值未定义，删除请求提交后也不能再用旧句柄查询。[到期查询契约](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/include/timers.h#L1315-L1331)

队列容量应由“daemon 来不及消费的窗口内可能出现多少请求”估计，计入任务突发、ISR 突发、回调自身提交的命令和事件组延后工作。容量更大能容纳更多积压，无法修复无限生产、永久自等待或超长回调。

daemon 的 CPU 预算还需包括活动链表操作、命令处理、到期回调和延后函数。提高 daemon 优先级不能代替对这些工作设置上界；栈容量要覆盖最深的回调调用链和实际配置下的上下文需求。日志本身也可能扰动测量，宜先设计轻量的时间记录。

软件定时器适合受控延迟的状态超时和周期性维护。若需求是准确的外设边沿、稳定采样时刻或严格硬件输出节拍，应根据设备能力设计硬件定时器、DMA 或受约束中断路径。即使使用硬件，后续任务何时处理数据仍是另一层延迟问题。本章没有提供某块板子的最坏延迟保证。

## 常见误区与失败边界

- “每个软件 timer 都有一个线程”：多个 timer 共享 daemon 的执行能力和栈
- “内部队列里装的都是已到期回调”：timer 操作命令与延后函数请求入队，正常到期来自活动链表
- “start/stop/delete 成功就是操作已完成”：成功首先是接收语义，完成要由协议确认
- “回调不是 ISR，阻塞无妨”：阻塞的是所有 timer 和延后工作共享的服务任务
- “等待参数为 0 就一定很快”：零等待避免等待空位，回调调用链中的计算和忙等仍需上界
- “one-shot 到期就自动销毁”：到期后 dormant，对象仍存在
- “auto-reload 超载会跳过旧周期”：本版本积压处理可能连续多次调用回调
- “插入函数 pdTRUE 表示插入成功”：此函数的 pdTRUE 表示应立即处理
- “active=false 就能 free ID”：快照不是在途回调、延后工作或应用消费者的寿命证明
- “停采集任务就停了它创建的 timer”：任务删除不会自动执行应用的 timer 与设备关闭协议

## 面试追问

1. **start 返回成功，active 查询却暂时为 false，矛盾吗？** 不矛盾。请求可能入队而 daemon 尚未处理；查询也只是一瞬间状态。先核对调用与处理时序，不能用查询替代完成通知。
2. **同一回调给十个 timer 使用，怎么知道哪个到了？** 参数给出 timer 句柄，ID 可关联稳定上下文。名称仅供调试；不要让十个 ID 指向反复复用的短寿命局部变量。
3. **为什么周期 10 tick 的 timer 连续调用三次？** 核对 daemon 是否迟到，以及自动重载的积压路径；区分三个计划点与三个现场开始时间。
4. **能在回调里 stop 自己吗？** 使用零等待并检查返回值；仍是向队列提交 stop，不能假设它立即终止当前积压调用。
5. **为何 ISR 置位的事件组也被慢 timer 拖住？** 固定版本将这类工作延后到同一 daemon，队列容量和 daemon 负载是共同依赖。
6. **把 daemon 优先级调到最高就有硬实时了吗？** 还需中断、临界区、调用链、积压与调度负载的可验证上界，优先级本身不提供证明。
7. **delete 后如何确认上下文能释放？** 先封住所有入口，再证明 daemon 处理完删除及旧延后工作，再确认应用消费者与硬件路径已静止。说明确认机制、失败路径和重启时如何防迟到事件。

## 动手练习与验收

以下均为**尚未执行的练习和验收标准**，不代表已有测试通过。

### 练习一 画命令和到期两条时间线

给一个周期 8 tick 的 one-shot：tick 30 调 start，tick 34 入队成功，tick 41 daemon 处理。再分别改为 reset 和 change-period。核对 start/reset 时间戳取得位置与 change-period 的处理基准。

验收：标出请求时间、等待空位时间、被接收时间、计划到期值和实际回调时间；不能统一从 tick 34 或 tick 41 起算所有 API。

### 练习二 重载欠账

一个 auto-reload timer 在 tick 50 启动，周期 7 tick；daemon 到 tick 73 才处理。手算欠下的计划点、该采样时刻对应的回调调用数量和下一次未来到期。再把回调耗时改成 9 tick，分析为何一次补处理仍可能引发下一轮迟到。

验收：区分内核的重载安排、回调调用数量和应用是否允许合并；不伪造历史采样数据。只报告推演结果，运行后才能报告实测结果。

### 练习三 回绕分支

用本章 8 位纸面模型重算表格，再增加 `c=248, P=12, n=3`。为每个输入写明返回 Boolean、是否立即处理以及选哪条链表。

验收：到期数值小于当前数值不必然意味着迟到；pdTRUE/pdFALSE 的解释必须与处理决策一致。实际工程另核对配置的 tick 位宽和观察间隔。

### 练习四 两种队列满

分别推演“任务发 start 时内部队列满”和“回调向应用队列投递工作时队列满”，再加入 ISR 的 reset 和事件组置位突发。

验收：每个队列分别列生产者、消费者、等待规则、失败返回和补救策略；ISR 与 daemon 回调不靠阻塞等待解决满队列。能解释哪个失败只影响本次请求，哪个关闭失败禁止释放对象。

### 练习五 删除竞态

枚举回调已开始、timer 已到期但未处理、stop/delete 待处理、延后事件组请求尚在队列、worker 正处理旧请求、重复 stop 和重启等事件。

验收：所有资源都有 live/owner/session 和最后访问点；没有事件访问 RELEASED 对象。确认标记入队失败或关闭超时时，资源不被强行释放。主机模型只能检查协议，不能证明某外设已停。

### 练习六 延迟记录

在后续可运行工程中记录计划点、callback 开始/结束和 worker 开始/结束；分别增加高优先级 CPU 负载、短周期 timer 数量和受控慢回调。

验收：报告 tick 频率、位宽、抢占配置、daemon 优先级、队列容量、栈配置和负载条件。将量化、调度等待、daemon 内部等待及 worker 延迟分开，不把一次平均值当成最坏上界。

## 源码阅读路线

按 [源码阅读方法](source-reading.md) 先找 API 契约，再跟控制流，不从逐行翻译开始。

1. `include/timers.h`：确认 handle、两种回调签名、创建参数、等待参数和返回值；跟 start/reset/change-period 的宏
2. `timers.c` 的 `prvInitialiseNewTimer()`：找周期、名称、ID 和链表项，画存储与拥有关系
3. `xTimerGenericCommandFromTask()` 与 `xTimerGenericCommandFromISR()`：画接收语义与队列满路径
4. `prvTimerTask()`、`prvProcessTimerOrBlockTask()`：找下一到期值和等待触发条件
5. `prvInsertTimerInActiveList()`：给每条返回路径写“现在处理”或“未来安排”，再核对回绕分支
6. `prvProcessExpiredTimer()`、`prvReloadTimer()`：核对 one-shot 状态和 auto-reload 积压调用
7. `prvProcessReceivedCommands()`：区分 start/reset 与 change-period 基准，核对 stop/delete 存储处理
8. `xTimerPendFunctionCallFromISR()` 及事件组延后函数：把 timer 和事件组画到同一个 daemon 图上
9. `tasks.c` 的调度器启动及所选 port：确认服务任务何时创建，ISR 调用边界由哪层约束

配套阅读：[内核链表](kernel-lists.md)、[任务生命周期](kernel-task-lifecycle.md)、[队列等待路径](kernel-queue-waits.md)、[事件组](kernel-event-groups.md)。基础复习：[RT02](rt02.md)、[RT06](rt06.md)、[RT10](rt10.md)；故障场景：[IC02](../120-interviews/ic02.md)。

## 来源与适用边界

- [FreeRTOS-Kernel V11.1.0 release](https://github.com/FreeRTOS/FreeRTOS-Kernel/releases/tag/V11.1.0)：公开版本入口；本章固定提交，不把滚动 main 当作相同实现
- [固定提交 timers.c](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/timers.c)：对象、命令队列、活动链表、daemon、到期与删除路径
- [固定提交 timers.h](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/include/timers.h)：公开 API 契约与宏
- [固定提交 tasks.c](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c)：调度器启动时创建 daemon；本章未据此宣称已验证具体目标上的全部调度延迟
- [官方 Kernel Book 第 6 章](https://github.com/FreeRTOS/FreeRTOS-Kernel-Book/blob/ba20b2f820926587d5ee20ce312b3789d63f795c/ch06.md)：共享 daemon 和短、非阻塞回调的概念说明；具体分支以本章固定版本源码为准
- [固定提交 event_groups.h](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/include/event_groups.h)与 [event_groups.c](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/event_groups.c)：ISR 延后置位/清位的关联路径
- [固定提交 GCC ARM_CM4F port](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/portable/GCC/ARM_CM4F/port.c)：本章采用的移植前提；其他架构需重新核对
- [V11.1.0 MIT 许可证](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/LICENSE.md)：上游内核的许可范围。若另行复制上游源码或实质性片段，需保留其版权与许可要求；本章没有复制加注释的内核实现

来源用于核对机制，不表示 FreeRTOS 或上游作者为本章背书。关闭确认标记、业务门控、工作交接和练习验收属于原创应用设计，需要在实际版本、配置、硬件和负载下另行验证。
