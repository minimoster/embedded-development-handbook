# FreeRTOS 内核链表 · 节点归属、排序与轮转

学习优先级：P1。先读 [源码导读](source-reading.md) 和 [RT01](rt01.md)。本章限定 FreeRTOS Kernel V11.1.0、单核 GCC ARM_CM4F；解释链表不变量和局部操作，未执行链表、内核或板级实验。

## 直接回答

FreeRTOS 的内核链表是带尾哨兵的双向循环链表。一个普通节点通过 `pvOwner` 找回所属对象，通过 `pxContainer` 记录当前所在列表。排序值决定有序插入的位置；游标决定单核轮转的下一项。一个节点同一时刻只能在一张列表中，但同一个 TCB 可以用两个不同节点同时表达状态与事件等待。

理解它时，先检查归属、双向链接、数量和游标，再讨论复杂度。链表函数不会自行建立完整并发保护，调用者必须满足所选内核路径的保护合同。[结构定义](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/include/list.h#L140-L179)、[链表实现](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/list.c)

## 原理拆解

### 1. 分清对象、节点与列表

| 位置 | 字段 | 阅读含义 |
| --- | --- | --- |
| `ListItem_t` | `xItemValue` | 由使用者定义的数值；可用于唤醒时刻或反向优先级编码 |
| `ListItem_t` | `pxNext`、`pxPrevious` | 循环中的前后邻接关系 |
| `ListItem_t` | `pvOwner` | 节点所嵌入的对象，任务路径通常是 TCB |
| `ListItem_t` | `pxContainer` | 当前容器；未挂入列表时为 NULL |
| `List_t` | `uxNumberOfItems` | 普通节点数，哨兵不计入 |
| `List_t` | `pxIndex` | 单核轮转游标，表示上次迭代所在位置 |
| `List_t` | `xListEnd` | 最大值哨兵，既标识边界又接起首尾 |

`configUSE_MINI_LIST_ITEM=1` 时，哨兵使用较小类型，只包含排序值、前后指针及可选检查字段；不能假定它拥有普通节点的 owner/container。关闭该选项后哨兵类型布局改变。`configUSE_LIST_DATA_INTEGRITY_CHECK_BYTES` 也会改变结构大小。[字段与布局](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/include/list.h#L102-L179)

owner 与 container 表示两种关系。例如 R 的事件节点 `pvOwner=R`，阻塞期间 `pxContainer=Q 的接收等待表`。醒来移除节点后，owner 仍是 R，container 则清为 NULL。移除操作不会释放 R，也不会自动取消外设持有的任务或队列句柄。

### 2. 空表与四条不变量

初始化列表后，哨兵前后都指向自身，游标指向哨兵，普通节点数为 0，哨兵值为 `portMAX_DELAY`。`vListInitialiseItem` 主要把节点 container 清为 NULL，不替调用者设置 owner、排序值或全部指针。[初始化](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/list.c#L49-L102)

在对象有效、没有中途并发修改的观察点，可以检查：

1. 对每个已挂入普通节点 I，`I.next.previous=I`，`I.previous.next=I`。
2. 从哨兵出发走一圈，恰好遇到记录数量的普通节点，并回到同一个哨兵。
3. 每个普通节点的 container 指向这张表；同一个节点不能重复挂入或同时挂入另一张表。
4. 游标指向本表的普通节点或哨兵，移除游标所在节点后不能留下悬空游标。

对采用 `vListInsert` 的有序表，再增加一条：从哨兵的 next 沿前进方向，普通节点值非递减。使用 `vListInsertEnd` 的轮转表没有这个通用排序保证。

### 3. 有序插入与游标插入是两种合同

`vListInsert` 按 `xItemValue` 升序查找位置；遇到相等值继续前进，因此新节点排在已有同值节点之后。待插值等于 `portMAX_DELAY` 时采用特殊路径，避免把最大值哨兵当作可无限前进的普通节点。[有序插入](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/list.c#L139-L211)

`vListInsertEnd` 不查排序值，直接把新节点放在当前 `pxIndex` 之前。游标在哨兵时，这相当于接到物理尾部；游标在普通节点时，位置要相对游标理解。名称中的 End 不能机械地读成“永远紧邻哨兵的前一个位置”。单核 Ready 列表使用这种轮转组织。[游标插入](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/list.c#L105-L136)、[加入 Ready 的宏](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L264-L274)

`uxListRemove` 已经收到节点地址，可沿其 container 和前后指针完成拆接。若游标正指向该节点，就先退到其前驱；随后清 container、减少数量并返回剩余数量。它不会把 owner 或所有旧链接统一清零。再次移除一个已经脱表的节点，或在仍挂入时直接重新初始化它，都会破坏调用前提。[移除](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/list.c#L215-L245)

### 4. 迭代不自动授予 CPU

单核 `listGET_OWNER_OF_NEXT_ENTRY` 推进游标，跳过哨兵，再取普通节点的 owner。调用者必须保证有可选普通节点。Ready 列表轮转只是候选顺序；何时调用选任务、是否请求切换，还取决于抢占、时间片、阻塞及 yield 路径。

本版本 SMP 分支没有这个单核游标宏，不能把相同解释直接搬到多核。[迭代宏](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/include/list.h#L285-L304)、[RT01 · 调度合同](rt01.md)

### 5. 逐个操作分析复杂度

下表只分析有效数据结构上的遍历与指针维护，忽略自定义 trace/hook、断言处理和同步等待。n 是该列表的普通节点数。

| 操作 | 相对 n 的结构开销 | 必要限定 |
| --- | --- | --- |
| 初始化一张表或一个节点 | O(1) | 不包含初始化所有优先级列表的外层循环 |
| `vListInsertEnd` / `listINSERT_END` | O(1) | 节点地址、列表和游标已经有效 |
| `uxListRemove` / `listREMOVE_ITEM` | O(1) | 已知且确实挂入的节点，不含按条件查找节点 |
| `vListInsert` | 最坏 O(n) | 升序找位置；特定插入点可能更早结束，最大值特殊分支固定开销 |
| 单核取下一 owner | O(1) | 非空列表；只推进有限次数，不含遍历全表 |
| 遍历并检查整张表 | O(n) | 要逐个访问节点 |

据此只能说“已知节点移除是固定结构开销”。延时表有序插入、多个到期任务处理、恢复调度时处理 pending-ready 都包含额外工作；队列项的复制也随项字节数变化。系统延迟分析需要这些操作所在路径的完整上界。

## 例子与推演

### 例一 同值顺序与计数

原创模型采用三个节点：A 值 9，B 值 4，C 值 9，依次有序插入。

```text
初始：S ↔ S                       数量 0
插入 A：S ↔ A9 ↔ S              数量 1
插入 B：S ↔ B4 ↔ A9 ↔ S        数量 2
插入 C：S ↔ B4 ↔ A9 ↔ C9 ↔ S  数量 3
移除 A：S ↔ B4 ↔ C9 ↔ S        数量 2，A.container=NULL
```

S 是哨兵。模型不为 S 取 owner，也不把数量 2 理解成三个物理结构中的两个字段。检查 C 的前驱是否仍指向已移除的 A，能比只检查计数更早发现拆接错误。

### 例二 轮转与新节点

Ready 列表已有 A、B，游标刚停在 A。把 C 插到游标之前后，继续迭代依次遇到 B、C、A。这个顺序来自游标关系；不能在不看游标的情况下声称 C 被加到“地址顺序的尾部”。如果先移除游标 A，移除函数退到 A 的前驱，下一次推进仍沿有效列表继续。

### 例三 一个任务的两个节点

R 带有限超时等 Q：状态节点在延时表，事件节点在 Q 的接收等待表。每个节点只有一个 container；R 这个对象同时被两张表引用。唤醒时还必须删除另一条等待关系，具体路径见 [任务生命周期](kernel-task-lifecycle.md) 与 [队列等待](kernel-queue-waits.md)。

## 常见误区与失败边界

- “双向链表插入/删除都是 O(1)”遗漏了有序查找位置；先区分已知位置插入与按值找位置。
- “container=NULL 就代表内存可释放”遗漏了 owner、应用句柄和其他生产者的引用。
- “加 volatile 就安全”不能替代内核保护范围，也不能修复重复挂入、悬空对象或非法 ISR 优先级。
- 完整性检查字节用于发现部分内存破坏；不开断言或没有覆盖到错误点时，不保证检测，开了也不能证明配置正确。
- 应用直接操作内核内部列表，可能绕过优先级位图、下一解阻塞时刻和事件合同。这里只把结构作为阅读对象。

## 面试追问

“为什么有 owner 还要 container？”答：前者找所属业务对象，后者让已知节点快速从当前表移除。两者分别服务身份与成员关系。

“高优先级为什么可能有更小的事件值？”答：普通有序队列等待表采用 `configMAX_PRIORITIES - uxPriority` 编码，使升序头部对应更高任务优先级。这个含义来自任务模块，不能推广到事件组借用节点值的路径。[事件节点初始化](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c#L1903-L1919)

## 动手练习与验收

练习尚未执行。用自己定义的 host 模型实现初始化、有序插入、游标插入和已知节点移除；不要复制内核源码作为自己的模型。

验收要求：

1. 每次修改后检查双向链接、数量、container、游标；空表与单节点也通过。
2. 输入同值、最小值、最大值；同值保持约定顺序，最大值不导致无限遍历。
3. 让模型主动拒绝重复插入、重复删除及删除哨兵。模型防护不等于官方函数承诺做这些检查。
4. 记录有序插入检查了多少节点，与 O(n) 上界对应；不能拿平均例子推成固定最坏时间。
5. 另外安排固定内核/port 的调试验证。host 模型通过只说明该模型满足其断言，不证明中断、内核集成或实时上界。

## 先修与复习入口

- [RT01 · Ready 与 Running](rt01.md)、[RT02 · 延时](rt02.md)、[RT05 · 有界队列](rt05.md)
- [源码导读](source-reading.md)、[任务生命周期](kernel-task-lifecycle.md)、[队列等待](kernel-queue-waits.md)

## 来源与适用范围

- [固定提交 list.h](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/include/list.h)、[list.c](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/list.c)：上述字段、操作与复杂度分析的实现依据。
- [固定提交 tasks.c](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/tasks.c)：Ready 轮转、有序事件值及调用前提。
- [对应 MIT 许可](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/LICENSE.md)：本文不转载内核代码；若后续分发源码或重要部分，应保留文件版权与许可声明。

正文、模型和练习为原创教学内容。来源用于核对机制，不表示源码维护者为本讲义背书；其他版本、SMP 与厂商修改内核需要重新检查。
