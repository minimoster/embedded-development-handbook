# FreeRTOS 源码导读 · 先固定版本，再追状态变化

学习优先级：P1。先能解释 RT01–RT10 的 API 合同，再用源码回答“这一步改了谁、为什么不会漏掉一次唤醒”。本章是补充阅读入口，不新增核心题编号。

## 直接回答

读 FreeRTOS 源码，可以从一条具体路径开始：任务等待空队列，ISR 放入数据，任务恢复运行。沿途观察队列计数、两个任务链表节点和移植层切换请求，就能把数据结构、阻塞条件与 CPU 上下文联系起来。先固定版本、配置和 port，避免把不同条件编译分支拼成一个不存在的内核。

本文及相邻源码章节以官方 FreeRTOS Kernel V11.1.0 为阅读基线，对应提交 `dbf70559b27d39c1fdb68dfb9a32140b6a6777a0`，限定单核 `portable/GCC/ARM_CM4F`。正文是独立编写的机制解释；伪代码、时序和练习为原创教学模型，未执行内核、模拟器或板级实验。[固定版本源码](https://github.com/FreeRTOS/FreeRTOS-Kernel/tree/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0)

## 原理拆解

### 1. 把阅读条件写在第一页

“FreeRTOS 使用某个结构”通常还不够。阅读前至少记录以下条件：

| 条件 | 本组章节的默认阅读假设 | 改动后要重查什么 |
| --- | --- | --- |
| 内核与核数 | V11.1.0，`configNUMBER_OF_CORES=1` | SMP 运行状态、锁与选任务路径 |
| 移植层 | GCC ARM_CM4F，非 MPU port | 首次启动、异常保存、临界区和 ISR 优先级 |
| 调度 | 抢占开启，时间片开启 | 合作式调度及同优先级 tick 行为 |
| tick | 32 位；频率由项目指定 | 回绕周期、量化、类型原子读合同 |
| 等待路径 | `INCLUDE_vTaskSuspend=1`；常规队列，不使用 queue sets | 无限等待落点、集合通知分支 |
| 互斥量与删除 | 对应功能开启时再读其分支 | 基础优先级、持锁计数、待回收列表 |
| 低功耗 | 主线先不采用 tickless idle | 下一解阻塞时刻、补 tick 与睡眠决策 |
| 内存与附加功能 | 按实际静态/动态配置；timer 等按需开启 | 创建失败、释放归属和服务任务 |

这是一份阅读假设，不是已编译通过的 `FreeRTOSConfig.h`。例如，ARM_CM4F 的优化选任务路径使用优先级位图和 CLZ，并要求优先级数量不超过 32；关闭优化后，通用路径会向下寻找非空就绪列表。不能把任一路径的局部开销推广成所有配置的调度开销。[选任务分支](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L159-L242)、[本 port 的类型与选择宏](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/portable/GCC/ARM_CM4F/portmacro.h#L49-L173)

### 2. 按责任看文件，不把所有同步压成一个队列

| 入口 | 优先观察 | 要带走的问题 |
| --- | --- | --- |
| `include/list.h`、`list.c` | 节点、哨兵、游标、容器 | 同一个节点现在属于哪张表？ |
| `include/task.h`、`tasks.c` | TCB、Ready/Delayed/Event 迁移、tick | 谁使任务不再具备运行资格？ |
| `include/queue.h`、`queue.c`、`include/semphr.h` | 项存储、计数、发送/接收等待列表 | 唤醒后为什么还要检查条件？ |
| `event_groups.c` | 条件位和等待者 | 多个任务的等待条件怎样判定？ |
| `timers.c` | timer 对象、命令队列、服务任务 | API 成功与命令处理完成是什么关系？ |
| `stream_buffer.c` | 独立缓冲结构、读写位置和通知路径 | 生产者/消费者合同与普通队列有什么差别？ |
| `portable/GCC/ARM_CM4F` | 栈初始化、SVC、PendSV、BASEPRI | 被选中的 TCB 怎样恢复成运行现场？ |
| `portable/MemMang` 与项目配置 | 分配器选择与对象寿命 | 创建和回收需要什么内存合同？ |

本版本的队列、信号量和互斥量共享 `Queue_t` 实现，但任务通知保存在 TCB 中；事件组有自己的条件位及等待列表；stream/message buffer 也有独立结构。共同使用任务阻塞机制，不代表具有相同的消息、容量和拥有权语义。[队列结构](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/queue.c#L68-L140)、[TCB 通知字段](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L358-L440)、[事件组结构](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/event_groups.c#L54-L66)、[缓冲结构](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/stream_buffer.c#L231-L252)

### 3. 用四个问题追一条调用路径

遇到内部函数，先查调用者，再回答：

1. 进入时有什么保护？任务临界区、ISR 屏蔽、调度器挂起、队列锁分别保护什么？
2. 改了哪个对象？数据项、计数、TCB 字段还是链表成员？
3. 离开时保留什么不变量？节点只属于一张容器，等待条件与列表相符，资源仍存活？
4. 返回值告诉调用者什么？复制成功、不能继续等待、需请求切换还是仅有命令被接受？

不要只读函数名。`xTaskRemoveFromEventList` 的 ordered-event 路径与事件组使用的 `vTaskRemoveFromUnorderedEventList` 有不同前提和迁移步骤。前者在调度器挂起时可暂存到 pending-ready；后者在其受控任务上下文中直接迁入 Ready，并延后切换。[两种解阻塞入口](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L5341-L5503)

## 例子与推演

从“优先级 4 的接收任务 R，优先级 2 的任务 W，空队列 Q”开始。先假定调度器未被外部挂起、Q 未锁定，ISR 优先级合规：

1. R 调用带有限等待时间的接收 API，发现 Q 为空。
2. 内核建立等待关系：R 的事件节点进入 Q 的接收等待表；状态节点离开 Ready，进入相应延时表。
3. W 得到运行机会。ISR 成功复制一项到 Q，更新计数并解阻塞一个等待者。
4. R 进入 Ready；ISR 的 `higherPriorityTaskWoken` 告知是否需要请求切换。
5. 本 port 通过 PendSV 保存 W 的现场、选择任务并恢复 R。R 回到接收路径后再次检查 Q，成功时才取走一项。

把第 3 步改成“调度器正在挂起”，再画一次：有序事件解阻塞可能先进入 pending-ready，待 `xTaskResumeAll` 完成迁移。把第 3 步改成“tick 先到截止点”，再画一次：任务先因超时获得运行资格，接收 API 的最终返回仍需看重新检查的队列条件。[接收路径](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/queue.c#L1502-L1649)、[事件迁移](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L5341-L5411)、[PendSV 与 tick](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/portable/GCC/ARM_CM4F/port.c#L505-L586)

建议每行只记一次状态变化：

```text
原创阅读记录格式，非内核代码
观察点 | 当前任务 | Q 的项数 | R 的状态容器 | R 的事件容器 | 切换请求
```

## 常见误区与失败边界

- 把源码中的一个字段当作稳定 ABI。TCB 和内部对象包含条件编译字段，不应由应用硬编码偏移或直接改表。
- 把链表操作当作天然原子操作。安全来自调用路径的保护合同；多次指针写入本身不会自动排除 ISR。
- 把挂起调度器当成关掉所有中断。ISR 仍可能更新对象，pending-ready 与队列延后唤醒就是要处理这类交错。
- 把高优先级任务进入 Ready 当成它已经完成接收或立刻运行。切换请求、调度机会、条件复查是不同步骤。
- 把读到源码当成目标通过验证。芯片中断分组、编译选项、FPU 使用、硬件寿命和最坏延迟仍需独立证据。

## 面试追问

“既然找任务很快，调度是不是全部 O(1)？”答：先指定操作。已知节点移除与本 port 的位图选择可以分析为固定结构开销；有序插入需要遍历，tick 可能处理多个到期任务，恢复调度可能补多个 tick。实时上界还取决于配置、对象数量、复制长度和被屏蔽时间。

“为什么还要读 port？”答：`tasks.c` 决定运行资格和选择，port 执行现场保存、异常返回与中断屏蔽。只画 Ready 列表无法解释 MSP/PSP、FPU 保存和 ISR 可调用范围。先复习 [AR07](../30-architecture/ar07.md) 与 [RT06](rt06.md)。

## 动手练习与验收

本章练习尚未执行。按以下次序建立证据，报告中分别写清：

1. 源码阅读：固定 commit、列配置、找到调用者及分支，填出一次空队列等待与唤醒记录。
2. 主机模型：独立实现简化状态机，枚举事件先到、超时先到、挂起期间到达。只验证该模型的断言。
3. 内核或模拟器：真正构建固定内核与选定 port 后，记录任务迁移、API 返回值和调度日志。主机模型不能代替这一步。
4. 目标板：核对 NVIC、向量表和存储寿命，用合适的计时方法测 ISR/切换/任务响应。测量结果只覆盖实际条件。

验收：每次 Ready↔Blocked 都能指出修改入口与两个节点的容器；每个时间结论有 tick 位宽、频率和配置；未执行的实验明确标为未执行。

## 先修与后续阅读

- [RT01 · 状态与调度](rt01.md)、[RT02 · 相对延时与绝对周期](rt02.md)
- [RT03 · 同步对象选择](rt03.md)、[RT04 · 优先级反转](rt04.md)、[RT05 · 通信语义](rt05.md)
- [RT06 · ISR 合同](rt06.md)、[RT07 · 三种保护范围](rt07.md)、[RT08 · 内存预算](rt08.md)、[RT10 · 创建与回调寿命](rt10.md)
- [内核链表](kernel-lists.md) → [任务生命周期](kernel-task-lifecycle.md) → [队列等待](kernel-queue-waits.md)
- [事件组条件](kernel-event-groups.md)、[软件定时器服务](kernel-timer-daemon.md)；需要历史比较时再读 [co-routines 附录](historical-co-routines.md)
- [IC01 · ISR 队列过载](../120-interviews/ic01.md)、[IC02 · 停止与对象释放](../120-interviews/ic02.md)
- [RT09 · SPSC 合同](rt09.md) 是独立并发模型，不能把其算法直接替换为内核链表或普通队列的证明

## 来源与适用范围

- [官方 V11.1.0 tag](https://github.com/FreeRTOS/FreeRTOS-Kernel/tree/V11.1.0) 与上述固定 commit：本组源码章节的实现基线。
- [FreeRTOS.h](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/include/FreeRTOS.h)：核数、tick 类型与可选功能需结合项目配置阅读。
- [V11.1.0 MIT 许可](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/LICENSE.md)：本文未复制内核源码片段。以后若分发源码或其重要部分，应保留对应文件的版权与许可声明。

正文、模型和练习为原创教学内容。来源用于核对机制，不表示源码维护者为本讲义背书；本章不提供 SMP、其他 port 或厂商修改版的通用保证。
