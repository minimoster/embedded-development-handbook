# FreeRTOS 队列等待 · 复制、解阻塞与超时复查

学习优先级：P1。先读 [RT03](rt03.md) 到 [RT07](rt07.md)，再看 [内核链表](kernel-lists.md) 与 [任务生命周期](kernel-task-lifecycle.md)。本章使用官方 FreeRTOS Kernel V11.1.0 固定提交 `dbf70559b27d39c1fdb68dfb9a32140b6a6777a0`，限定单核 GCC ARM_CM4F、常规队列且不启用 queue sets；其他默认条件见 [源码导读](source-reading.md)。所有时序与练习均为原创推演，未执行内核、模拟器或板级实验。

## 直接回答

队列把固定大小的数据项复制进有界存储，并用两张事件等待表记录“等空间的发送者”和“等数据的接收者”。一次成功发送可解阻塞接收者，成功接收可解阻塞发送者；被解阻塞的任务还要重新获得 CPU、复查条件，才能实际完成操作。

本版本的二值/计数信号量与互斥量复用 `Queue_t`，但不复制普通数据项。互斥量另记录持有者和递归次数，并接入简化优先级继承。这个实现关系只覆盖这几种对象，不能推成所有 FreeRTOS 同步对象都具有队列语义。[Queue_t 与专用字段](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/queue.c#L56-L140)

## 原理拆解

### 1. 存储与等待关系分开读

| 字段 | 含义 | 需要检查的边界 |
| --- | --- | --- |
| `uxLength` | 最多容纳多少项 | 有限容量，不是字节数 |
| `uxItemSize` | 每项复制的字节数 | 指针项只复制指针字节；信号量项大小为 0 |
| `uxMessagesWaiting` | 普通队列当前项数，或信号量可用计数 | 不越过容量上限，不把满当成功 |
| `pcHead`、`pcWriteTo`、队列联合体中的指针 | 数据存储和读写位置 | 在所分配区内推进并回绕 |
| `xTasksWaitingToSend` | 等待可发送条件的任务事件节点 | 按普通事件优先级编码排序 |
| `xTasksWaitingToReceive` | 等待可接收条件的任务事件节点 | 解阻塞后不直接保留某个数据项 |
| `cTxLock`、`cRxLock` | 等待列表更新被延后时的锁信息 | 是内部机制，不能当作应用互斥量 |

动态普通队列在该实现中把控制结构和数据存储放在一次分配的区域内；静态创建使用应用提供的控制块和存储。概念上分为两类空间，不代表动态实现必然执行两次 malloc。复制路径使用项字节数推进读写位置。[创建](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/queue.c#L502-L610)、[数据复制](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/queue.c#L2390-L2491)

例如长度 4、项为 12 字节的普通队列最多保存 4 项数据，其项存储为 48 字节；还要单独计控制结构等开销。若改成 4 个指针项，空间按目标指针大小计算，目标缓冲的寿命、拥有权与归还仍属于应用合同。发送成功不授权提前复用目标。[queue.h 合同](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/include/queue.h#L89-L111)、[RT05 · 指针寿命](rt05.md)

### 2. 三种专用对象怎样复用表示

| 对象 | 主要计数语义 | 额外合同 |
| --- | --- | --- |
| 普通数据队列 | 0…容量的已保存项数 | 每项按值复制 |
| `xSemaphoreCreateBinary` 创建的二值信号量 | 0 或 1，初始为 0 | 先 give 才可 take；没有 mutex 的拥有权与继承 |
| 计数信号量 | 0…最大计数，初始计数由创建参数给出 | 连续 give 有上限，溢出需要处理 |
| 普通互斥量 | 初始可取得，持有期间不可取得 | 持有任务必须释放；记录 holder 与持锁计数 |
| 递归互斥量 | 还记录同一拥有者重复取得次数 | 使用配套递归 take/give API，次数必须匹配 |

信号量的零字节项不代表没有状态或没有内存需求。mutex 创建使用长度 1、项大小 0 的队列基础，再初始化 holder 与递归计数，并建立可取得状态。这里的 `pcHead=NULL` 被内部实现用作 mutex 类型标记，不能套普通数据区指针的解读。[专用初始化](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/queue.c#L614-L682)、[计数创建](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/queue.c#L900-L934)、[二值与递归合同](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/include/semphr.h#L110-L167)、[递归 API 合同](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/include/semphr.h#L303-L394)

旧二值创建宏的初始状态不能移植成上述新创建 API 的状态。应用应遵守公开 API，不应通过改内部计数字段模拟 give/take。

### 3. 常规发送与接收的两层检查

先看最短路径。在短临界区内：

- send 发现有空间，就复制一项、更新数量，并检查接收等待表；满且等待参数为 0，则失败返回。
- receive 发现有数据，就复制一项到调用者提供的空间、减少数量，并检查发送等待表；空且等待参数为 0，则失败返回。

需要等待时，调用会记录超时基准，退出最初的临界区，再挂起调度器并锁定队列的等待列表更新。随后检查剩余等待预算，重新检查满/空条件，确实仍不能完成时才建立事件与时间等待关系。[发送路径](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/queue.c#L939-L1154)、[接收路径](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/queue.c#L1502-L1649)

第二层检查很重要：第一次发现空/满，到真正准备阻塞之间，ISR 或其他任务可能已经改变条件。只凭第一次观察就睡下，会漏掉已经发生的进展。

下面是原创概念伪代码，省略具体 API、锁顺序和错误码，不能编译为 FreeRTOS 替代实现：

```text
带预算的接收模型
反复执行：
    在受保护的观察点，若有项：取一项并返回成功
    若没有等待预算：返回失败
    建立或更新剩余预算
    在允许建立等待的保护条件下，再检查队列
    若仍空且还能等：登记事件条件和截止条件，交出运行机会
    恢复执行后，回到观察点
```

这段模型只强调复查。实际发送、接收、信号量以及可选中止等待分支必须分别看固定源码。

### 4. 队列锁延后的是等待表处理

队列处于内部 locked 状态时，合规 ISR 仍可以在自己的保护路径中增加或取走数据；它先记录相应锁信息，暂不按通常路径改事件等待表。`prvUnlockQueue` 根据这些变化解阻塞等待者。此时调度器仍挂起，有序事件路径可能把事件节点暂存到 pending-ready，随后由 `xTaskResumeAll` 完成状态迁移。[解锁循环](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/queue.c#L2494-L2613)、[pending-ready 迁移](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L3956-L4103)

因此不能把“队列锁定”理解成所有访问者停止，也不能把它理解成数据区自动对任意高紧迫性 ISR 安全。该机制依赖内核 API 和 port 规定的可调用中断范围。锁字段也不是无界事件统计器，业务丢弃统计应另行设计。

### 5. 解阻塞、条件满足与最终返回

接收等待表选出一个任务，只使它有机会再次检查队列，不为它预留某一项。另一个更高优先级接收者可以先消费；原等待者重新运行时可能还要继续等。发送者获得的空槽也不是应用已拥有的永久名额。

`xTaskCheckForTimeOut` 根据进入时刻、溢出计数和已过去 tick 更新剩余预算。一次醒来后再次阻塞不会重新获得完整原始等待时长。[超时预算更新](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L5506-L5600)

特别留意本版两条路径的差别：

- `xQueueReceive` 检测到超时后，先解锁并恢复调度，再检查队列是否仍空；若此时非空，会返回循环尝试接收，不能直接等同“超时路径必然失败”。
- 常规 `xQueueGenericSend` 一旦进入其检测到超时的分支，解锁并恢复调度后直接返回失败，没有接收路径同样的最终满/空复查。
- 两者循环顶部都先检查当前可完成条件。任务因截止而 Ready 后，若再次运行时条件已满足，可能在顶部直接完成。因此“等待 5 tick”不是业务动作必定在精确截止时刻失败或结束的实时合同。

比较的是实际走到的源码分支，不是只比较日志里的 tick 值。[接收超时分支](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/queue.c#L1629-L1649)、[发送超时分支](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/queue.c#L1142-L1154)

若业务要求严格截止，例如“超过时限的响应一律不使用”，需要用明确时钟与应用协议检查消息的时间、序列和有效期；不能让一个阻塞参数承担未声明的业务时效保证。

### 6. FromISR 是专门合同

`xQueueSendFromISR` 和 `xQueueReceiveFromISR` 不含可阻塞的等待时间。队列满/空时返回失败，ISR 不能等待消费者或生产者运行。数据复制与正常寿命规则仍适用，大项复制也会增加 ISR 执行时间。[发送 ISR 路径](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/queue.c#L1157-L1326)、[接收 ISR 路径](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/queue.c#L2035-L2132)

每次 ISR 入口将共享的 `higherPriorityTaskWoken` 标志初始化为 false；同一次 ISR 中多个合规操作可累计这个标志，最后按本 port 的 `portYIELD_FROM_ISR` 请求切换。API 设置标志表示解阻塞了比被中断任务优先级更高的任务，不表示这个任务已经执行或消息已经处理完成。

ARM_CM4F 的临界区使用 BASEPRI。ISR 必须处于允许调用内核的紧迫性范围，实际优先级编码、硬件实现位数和分组都需核对；数字较小的更高紧迫性中断不能随意访问内核。启用 `configASSERT` 有助于发现违规，但不自动替项目配置正确优先级。[本 port 屏蔽与 yield 宏](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/portable/GCC/ARM_CM4F/portmacro.h#L87-L125)、[优先级校验](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/portable/GCC/ARM_CM4F/port.c#L851-L909)

二值/计数信号量使用明确适用的 FromISR give/take API。mutex 及递归 mutex 不能在 ISR 中使用；函数族共用底层结构不授权跨越拥有权与继承合同。[信号量 ISR 合同](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/include/semphr.h#L553-L579)、[take-from-ISR 合同](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/include/semphr.h#L642-L673)

### 7. 简化继承的边界

任务取得 mutex 时记录 holder 并增加其持锁计数。较高优先级任务因 mutex 不可取得而需要阻塞时，等待路径调用继承函数；holder 的有效优先级可能提高，若它在 Ready 列表中，就迁到新优先级的 Ready 列表。继承不会把一个仍因其他原因阻塞的 holder 自动变成可运行。[mutex take](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/queue.c#L1652-L1817)、[继承](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L6580-L6676)

V11.1.0 的释放回退路径只有在 holder 不再持有其他 mutex 时，才回到基础优先级。等待者超时导致的回退路径则有“holder 只持有一个 mutex”的简化条件，再结合基础优先级与该 mutex 其余最高等待优先级决定目标值。多锁场景可能保留继承优先级更久；不能宣称每释放一把锁就立即精确重算全部依赖图。[释放回退](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L6683-L6770)、[超时回退](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L6777-L6892)

继承没有解决 ABBA 死锁、无界持锁、在锁内等待慢 I/O 或删除持锁任务的资源归还。锁序、有限临界区和退出协议继续由应用负责，复习 [RT04](rt04.md)。

## 例子与推演

### 例一 被唤醒的数据先被别人取走

原创条件：R1 优先级 3 等空 Q，P 优先级 1 正运行；R2 优先级 4 暂时阻塞在另一个条件上。

1. ISR 给 Q 放入一项，解阻塞 R1。
2. 在 R1 真正执行接收前，另一个合法事件使 R2 Ready；R2 先运行并取走这项。
3. R1 再运行时看到 Q 为空。若还有预算，重新建立等待；否则走失败判断。

这不是队列丢了一项。原等待者没有取得数据拥有权，解阻塞也不为它做事务预留。

### 例二 截止时刻与最终返回

R 在 tick 40 调用等待 5 tick 的接收。tick 45 先使 R 因时间到期 Ready；优先级更高的 P 随后发送一项，再让出运行机会。R 回到循环顶部看到数据，仍可成功。另一个分支是 R 先看到队列空并沿失败路径返回，数据随后才到达。这两个结果都需要记录真实处理顺序，不能只写“tick 45 到了”。

### 例三 持有两把锁时的回退

L 基础优先级 1，已持有 A、B 两把 mutex；H 优先级 4 等 A，使 L 继承到 4。L 释放 A 后仍持有 B，按本版释放路径可继续保持 4；释放最后一把 B 时再恢复到 1。H 因 A 可用进入 Ready，并不自动保证它在 L 释放 A 的那一行立即独占 CPU；还要分析同优先级候选、yield 和时间片。

## 常见误区与失败边界

- “内核队列安全，所以指针目标安全”遗漏应用缓冲拥有权、DMA 活动和迟到回调。
- “等待者先来就先获得数据”遗漏优先级排序、实际调度与条件再次变化。
- “超时处理与事件处理固定谁优先”忽略具体函数分支、保护窗口与 pending-ready 恢复顺序。
- “信号量满时 give 也算一次完整事件”遗漏容量限制；二值状态可合并事件，计数也会到上限。
- “队列删除会清理所有指针项和访问者”错误。动态删除释放内核队列存储；静态存储由应用管理；被复制的指针目标不会因此被释放，ISR、任务和回调也不会自动停止。[删除分支](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/queue.c#L2254-L2298)

队列已无内核动态存储需要释放，也不代表可以在仍有访问者时调用删除或复用静态存储。先完成 [IC02 的停止与静止协议](../120-interviews/ic02.md)。

## 面试追问

“一次 send 的复杂度是什么？”答：指定路径。数据复制随项字节数变化；有空间时无需扫描整个数据队列；需要阻塞时事件表和延时表的有序插入会遍历；队列解锁还可能处理多个等待者。因此不能把所有发送与唤醒路径统一写成 O(1)。

“ISR 满队列怎么处理？”答：检查返回值，根据业务选择可观测丢弃、背压或更合适的缓冲方式。统计、恢复与指针归还必须明确；不能在 ISR 忙等接收任务。参见 [IC01](../120-interviews/ic01.md)。

## 动手练习与验收

练习尚未执行。分别保存源码阅读、host 模型、内核/模拟器和目标板证据：

1. 长度 2 的队列：输入短突发、满队列与空队列；每次 API 返回与已复制项数一致。
2. 等待模型：枚举第一次空检查后数据到达、登记等待后到达、截止先处理、醒来后被其他接收者取走；每次重新阻塞使用剩余预算。
3. ISR 模型：枚举队列锁定与未锁定、有 pending-ready 与无 pending-ready；不会因锁定而假定数据访问停止。
4. mutex 推演：单锁释放、多锁释放和等待者超时分别核对 holder、基础/有效优先级、持锁计数。不要把单锁结论推广到所有多锁图。
5. 停止模型：旧 ISR、旧会话指针、重复 stop、静态与动态对象各有寿命验收；超时仍有访问者时不能强行释放。

验收：没有静默丢项、没有提前复用指针目标、没有重复入 Ready；满/空/超时都有业务处理。host 模型不证明 FreeRTOS 实现或 ISR 安全；真正运行固定内核/port 后才能报告对应验证，目标中断优先级、耗时与硬件静止还需板级检查。

## 先修与复习入口

- [RT03 · 对象选择](rt03.md)、[RT04 · 反转与死锁](rt04.md)、[RT05 · 通信语义](rt05.md)
- [RT06 · ISR 优先级](rt06.md)、[RT07 · 保护范围](rt07.md)
- [源码导读](source-reading.md)、[内核链表](kernel-lists.md)、[任务生命周期](kernel-task-lifecycle.md)
- [IC01 · 过载处理](../120-interviews/ic01.md)、[IC02 · 对象寿命](../120-interviews/ic02.md)

## 来源与适用范围

- [固定提交 queue.c](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/queue.c)、[queue.h](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/include/queue.h)、[semphr.h](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/include/semphr.h)：表示、等待、复制与公开 API 合同。
- [固定提交 tasks.c](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c)：预算、事件表迁移与简化继承。
- [固定提交 ARM_CM4F port](https://github.com/FreeRTOS/FreeRTOS-Kernel/tree/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/portable/GCC/ARM_CM4F)：临界区、yield 与 ISR 优先级校验。
- [对应 MIT 许可](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/LICENSE.md)：本文未转载内核代码；以后分发源码或重要部分应保留对应版权与许可声明。

正文、模型和练习为原创教学内容。来源用于核对机制，不表示源码维护者为本讲义背书；queue sets、SMP、其他 port 和厂商修改版需单独核对。
