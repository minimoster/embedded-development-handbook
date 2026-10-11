# 可选合同补充 · 队列操作、调试名称与 queue sets

学习优先级：P2。先读 [RT05](rt05.md) 和[队列等待主线](kernel-queue-waits.md)。本章只补操作选择与诊断合同，使用官方 FreeRTOS Kernel V11.1.0 固定提交 `dbf70559b27d39c1fdb68dfb9a32140b6a6777a0`，单核 GCC ARM_CM4F。普通队列部分不启用 queue sets；最后一节单独说明启用它时新增的条件。没有执行内核、模拟器、板级或 host 队列模型。

## 直接回答

send-to-front 改变入队位置；overwrite 是长度一队列的“最新值”合同；peek 不消费项；reset 把既有普通队列清空，却不是取消所有任务或释放指针目标。调试 registry 只把句柄与长寿命名称关联。queue set 则提供多个成员的就绪事件选择，不能把这些机制合成一个任意读取、自动拥有资源的接口。

## 四种普通操作的原创推演

设队列长度为三，当前取出顺序为 A、B。

| 操作 | 结果/边界 | 应用仍需处理的事 |
|---|---|---|
| send-to-back C | 成功后 A、B、C | 满时等待/失败政策仍适用 |
| send-to-front C | 成功后 C、A、B | 前插不改变任务优先级；成功发送可能唤醒更高优先级接收者并按调度条件抢占；反复前插可推迟旧项 |
| peek A | 复制 A，队列仍是 A、B | 不是预留；其他消费者可能先取走 A |
| reset | 项数归零，存储仍存在 | 被丢弃的指针目标如何归还，需由独立所有权协议解决 |

以上讨论数据项顺序，不把事件等待者顺序与数据顺序混为一谈。复制目标、项宽、等待预算和错误返回仍按[队列等待](kernel-queue-waits.md)的合同核对。[task send 路径](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/queue.c#L936-L1146)、[copy-in](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/queue.c#L2390-L2491)

`xQueueOverwrite` 只用于长度一队列：空队列写入后项数为一，满队列覆盖后仍为一，不增加历史消息。它适合合成的“最新测量值”示例，不适合每次都必须处理的订单/告警记录。被覆盖的数据若含拥有型指针，内核不会自动释放旧目标；先定义资源回收办法。断言并不能把任意长度队列的错误调用变成合法用法。ISR 要使用对应、明确允许的 FromISR API，并遵守[RT06](rt06.md)的优先级/唤醒合同。[overwrite 公开合同](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/include/queue.h#L515-L603)、[实现边界](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/queue.c#L950-L984)

## reset 不是重新初始化所有等待表

本版本普通队列的 reset 设置项数和环形读写位置，保留已经存在的等待关系：接收者仍等数据；若有等待发送者，内部路径可解阻塞其中一个，再由调度条件决定运行。新对象初始化才初始化两张等待表。不要把内部 `xNewQueue` 标志当应用参数，也不要用 reset 模拟 mutex 解锁、对象销毁或应用取消广播。[reset 路径](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/queue.c#L303-L369)

例如停止采集时，先静止生产者/ISR/DMA，决定未消费对象的回收者，再唤醒或取消消费者并确认退出。先 reset 再希望所有阻塞任务自行退出，会同时漏掉等待者和指针目标的寿命问题。本节不提供 queue set 成员 reset 的事件账务协议。

## registry 是调试索引，不是对象寿命证明

启用 `configQUEUE_REGISTRY_SIZE > 0` 后，registry 用于内核感知调试器查找队列、信号量或 mutex。上限只限制登记槽位，不限制可以创建多少对象；登记 API 返回 void，满表没有应用层成功回执。对同一有效句柄重新登记可更新名称。

这里保存名称指针，不复制字符串。名称必须覆盖整个登记/读取期，不能用退出后失效的局部字符数组。`pcQueueGetName` 的返回值是登记名称指针；没有名字不是创建失败，有名字也不是并发使用或目标对象仍活着的证明。读/更新/取消登记应与应用生命周期和调试操作协调，不当作线程安全的业务服务发现。动态/静态对象的删除和缓冲重用仍需要静止协议。[registry 实现](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/queue.c#L3007-L3127)、[名称寿命合同](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/include/queue.h#L1536-L1608)

## 独立选读 · queue sets 增加一份事件账

只有项目确实需要等待多个队列/信号量时再考虑 `configUSE_QUEUE_SETS = 1`。本版本 `xQueueCreateSet` 公开创建入口还依赖动态分配配置，失败返回 NULL。已有队列等待主线的“不启用 queue sets”假设不因本节而改变。[创建与成员路径](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/queue.c#L3174-L3268)

set 内部保存成员句柄事件，不把所有成员载荷搬到一个大队列。对普通入队，成员增加可消费项时会通知 set；长度一队列在已有项上 overwrite 并没有新增项，也不会因此再增加一个 set 事件。容量与额外内存必须按成员容量和目标指针宽度计算，不能把文档的 32 位平台示例“每项 4 字节”推成所有目标常量。[通知路径](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/queue.c#L971-L1002)、[set 元素大小](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/queue.c#L3174-L3189)

一个原创预算：成员队列容量二、另一队列容量四、一个二值信号量容量一，事件队列按至少七项核算，再计各对象控制块、项存储和应用载荷。这个和式建立在遵守成员使用协议的前提上；平均流量不是容量证明。

成员加入时应为空且尚不属于其他 set；移除要求属于该 set 且为空，但仍需让生产者静止以免竞态。使用协议是先 select，得到某成员句柄，再从那个成员 receive/take。不能另起一个消费者直接绕过 set 任意消费，也不能把一次 select 当成可长期保存的资源预留；否则成员项与事件记录可能失配。若确需多个消费者，必须额外设计并验证消费所有权，而不是只加一把无关 mutex。[成员 API 条件](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/queue.c#L3192-L3305)、[公开选择合同](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/include/queue.h#L1631-L1778)

等待包含 mutex 的 set 不会因为这一等待自动使 mutex 持有者继承等待者优先级；它不同于直接阻塞在 mutex take。具体应用通常先考虑更简单的消息队列、通知或明确的调度设计，再选择 set，避免把“可等待多个对象”当成自动解决所有权、继承和关闭。[queue sets 注意事项](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/include/queue.h#L1631-L1669)

## 动手练习与验收

纸面逐步推演 front/back/peek/overwrite/reset 的项数、顺序、等待者和资源回收者；再给 set 画成员项与事件的两份账，注入绕过 select 的直接消费、满 set、重复前插、旧指针覆盖和关闭竞态。每条失败路径说明如何恢复或拒绝，不把查询结果当预留。

上述练习尚未执行 host 模型或固定 FreeRTOS 内核。需要项目采用这些操作后，在确切配置/port 下分别验证 API 返回、事件账、ISR 规则、资源寿命和时延；源码核读与手推不等于内核测试通过。

## 来源与适用范围

固定源码的 [queue.c](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/queue.c)、[queue.h](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/include/queue.h)属于对应的[官方 MIT 许可](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/LICENSE.md)。本文没有转载内核实现代码，正文、例子和练习为原创；分发源码或重要部分需保留对应版权与许可。其他版本、SMP、port 或厂商修改不能只凭本章推断。
