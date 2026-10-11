# 可选深化 · Linux DMA 的所有权、取消与 remove 生命周期合约

学习优先级：P1，可选深化。先完成 [DV05](dv05.md)，再按岗位需要深入本章。Linux 驱动岗、BSP 岗和涉及异步 I/O 的岗位建议掌握；首次复习不必背完所有状态。

本章是原创设计推演与有限主机实验，不是公司题库、可加载内核模块或板测报告。核对日：2026-10-11。API 基线固定为 Linux **v6.6**；更新内核、厂商树或更换 DMA 控制器后，重新核对实现与能力。

## 1. 这章补哪一个洞？

“map → DMA → unmap”只能描述正常路径的一部分。工程中更容易出事故的问题是：

- 映射成功但描述符准备失败，谁回收映射？
- 描述符已 submit，但还没 issue，remove 能否直接 free？
- 等待超时后，设备是否仍可能访问 buffer？
- 回调已经唤醒等待者，是否等于回调已经离开 request 对象？
- `terminate_async()` 返回后，什么时候才有资格 unmap/free？
- remove 与新请求相遇时，谁关闭入口，谁排空在途引用？
- 停止操作返回错误时，是否还能宣称“资源已安全释放”？

[IC04](../120-interviews/ic04.md) 已讨论地址空间和基本 streaming 所有权；本章把“最后必须先停 DMA”拆成能审查、能反例测试的具体合约。[MC06](../40-mcu/mc06.md) 的 Cortex-M7 cache 维护属于另一套目标硬件规则，不把它直接搬进 Linux DMAengine 客户端。

## 2. 面试分层答案

### 30 秒：先守住释放的必要条件

我会把数据 buffer、映射、描述符和回调上下文的生命周期分别记账。Streaming 映射成功后，CPU 不在设备拥有窗口随意访问；submit 后客户端不再使用描述符指针。超时只是等待者没等到，不能作为 DMA 已停止的证据。remove 先关闭新提交入口，再停止并同步 DMA 和回调，最后按原映射参数 unmap/free。异步 terminate 后还必须 synchronize；停止失败就保留并隔离相关资源，不能继续假装安全回收。

这段答案的核心不是多说几个 API 名字，而是明确：**谁仍能访问什么，以及什么事件证明这个访问权已经结束。**

### 2 分钟：说明本次采用的设计

我限定为独占 slave DMAengine channel、一次性 SG 收发、每次请求短寿命映射，不做 cyclic、不复用描述符、不在回调重提请求。

映射所用的 device 来自 `dmaengine_get_dma_device(chan)`，不因为 request 属于某个外设，就随手使用外设的 device。`dma_map_sg()` 返回 0 是失败；成功返回的 DMA 段数用于 prep，原始 SG 项数用于 unmap，两个数不能混用。映射方向与传输方向分别使用对应枚举。[v6.6 DMAengine client guide](https://www.kernel.org/doc/html/v6.6/driver-api/dmaengine/client.html)、[v6.6 DMA API](https://www.kernel.org/doc/html/v6.6/core-api/dma-api.html#part-id-streaming-dma-mappings)

准备成功后设置回调，并紧接着 submit；提交成功仅表示排队，issue_pending 才启动待处理工作。门禁与这组操作需要真实同步，避免 remove 关门后又冒出一笔请求。本章把 prepare/submit 视作不被 remove 插入的短临界段。

超时或 remove 的进程上下文可以采用成功的 `dmaengine_terminate_sync()` 作为 DMA 与回调静默的依据。原子上下文或回调内只能采用适用的异步方案，再交给可睡眠上下文 synchronize；两者之间禁止 issue_pending。清理负责人只走一次释放路径；取消后的不完整请求不会靠“等完成回调”自然收尾。[v6.6 固定源码：终止与同步合约](https://github.com/torvalds/linux/blob/v6.6/include/linux/dmaengine.h#L1054-L1129)

### 深挖：不能把五种“完成”混为一个布尔值

1. **调用者结束等待**：超时、信号或取消可以结束一次等待，设备可能仍在运行。
2. **DMA 搬运停止/结束**：内存访问结束；回调可能待调度或仍在运行。
3. **CPU 访问权恢复**：短映射完成 unmap，或在另一种复用设计中完成适当 sync。
4. **回调与其他软件使用者静默**：request、completion、工作项、外设状态对象等不能再被旧使用者访问。
5. **业务数据完整**：DMA 停止不代表接收帧完整、FIFO 排空、发送线已空或校验通过。

本章模型主要检验第 1～4 项中的一小部分。第 5 项只模拟成功/错误结果标签，不能推断外设上的真实帧边界。

## 3. 明确范围，避免拿一种驱动代替所有 Linux DMA

### 3.1 设计声明

| 项目 | 本章选择 | 必须另行设计的情况 |
|---|---|---|
| DMA 使用方式 | 独占 slave DMAengine channel | 网卡自行管理硬件 ring、GPU、块层、async_tx、共享调度 |
| 请求 | 一次性 SG，最多一笔逻辑请求 | cyclic、多个并发 request、流水线、回调链重提 |
| 映射 | 每笔请求一次成功 map，最终一次 unmap | 长寿命映射的 CPU/device sync 循环 |
| 客户端 buffer 访问 | map 后直到 unmap 不访问 payload | 在已证明 DMA 静止的区间 sync 后访问 |
| 描述符 | 不使用 `DMA_CTRL_REUSE` | provider 明确支持的描述符复用合约 |
| 回调 | 只处理结果/通知，不重新提交 | callback 可重提的 slave/cyclic 专门设计 |
| 同步 | 主机模型把外部事件串行排列 | 真正的锁、引用计数、工作队列及多核可见性证明 |
| 故障恢复 | 成功停止后清理；失败时保留 | 控制器 reset、IOMMU、总线级断开和硬件热拔除 |

DMAengine 文档允许某些 slave/cyclic 回调准备并提交下一笔；本章主动缩窄策略，不能把模型的限制误读为 Linux 禁止这种行为。[固定版 client.rst](https://github.com/torvalds/linux/blob/v6.6/Documentation/driver-api/dmaengine/client.rst)

### 3.2 先确定真正执行 DMA 的设备

slave 客户端有“消费服务的外设”和“提供 DMA 的控制器”。映射、适用的 cache sync、unmap 必须使用同一正确 DMA device。获取它的入口是 `dmaengine_get_dma_device(chan)`；v6.6 helper 还能按通道选择不同的映射 device，所以不应自己用一个结构体成员猜出答案。[v6.6 helper](https://github.com/torvalds/linux/blob/v6.6/include/linux/dmaengine.h#L1543-L1549)

因此，基础讲义中的“检查 DMA 寻址能力”不能在这里简化为“客户端任意改控制器 mask”。真正 DMA master 的寻址能力、DMA ops、固件配置和 controller driver 的职责都要核对；客户端不能为了让某次 map 成功就悄悄扩展硬件并不支持的能力。

本章不决定 FIFO 地址、总线宽度、burst、外设流控、cache line 对齐或可映射内存的具体分配方案；这些是驱动接入目标平台前必须完成的输入条件。`vmalloc` 指针、栈 buffer、CPU 指针强转地址都不是现成的可移植 DMA 输入。[v6.6 DMA HOWTO](https://www.kernel.org/doc/html/v6.6/core-api/dma-api-howto.html#what-memory-is-dma-able)

## 4. 三份账：映射账、描述符账、访问者账

### 4.1 映射账：成功 map 才产生对应 unmap 的责任

本章用 SG 而非 `dma_map_single()`，因为“原始项数”与“映射后段数”是常见的生命周期记账错误。

假设 request 建立了 3 项 SG，映射后返回 2：

- `orig_nents = 3`：输入 SG 的项数，必须保留至清理。
- `dma_nents = 2`：设备侧可使用的 DMA 段数，用于 `dmaengine_prep_slave_sg()`。
- unmap 使用 `orig_nents = 3`；若做 SG sync，也保存并使用原始项数。
- `dma_map_sg()` 返回 0：没有本次成功映射，不提交，不制造一笔假 unmap。
- 已映射的 SG 不重复 map；本章 map 成功后，直到 unmap 才返回 CPU 拥有窗口。

这些参数是合约的组成部分，不是“试两个数字，哪个不崩用哪个”。[v6.6 DMA API：SG 与 sync 参数](https://github.com/torvalds/linux/blob/v6.6/Documentation/core-api/dma-api.rst#L342-L428)

映射 API 的失败判定也不通用：`dma_map_single()/dma_map_page()` 用 `dma_mapping_error()`；SG 用返回 0。不要把错误地址与 0 比较当成单 buffer API 的通用检查。[v6.6 DMA HOWTO：错误处理](https://www.kernel.org/doc/html/v6.6/core-api/dma-api-howto.html#handling-errors)

### 4.2 两种方向，两个枚举

| 业务 | 映射/解除映射的方向 | DMAengine prep 的方向 |
|---|---|---|
| 内存发送给外设 | `DMA_TO_DEVICE` | `DMA_MEM_TO_DEV` |
| 外设接收到内存 | `DMA_FROM_DEVICE` | `DMA_DEV_TO_MEM` |

不要因为名字接近或某个平台的整数值碰巧相关，就直接传错枚举。v6.6 `dmaengine_prep_slave_sg()` 的参数类型是 `enum dma_transfer_direction`。网页文档中的某些示意片段并非可直接编译的完整函数调用；核对头文件，补齐映射 direction，不能照抄漏参数的示例。[v6.6 实际 prep helper](https://github.com/torvalds/linux/blob/v6.6/include/linux/dmaengine.h#L932-L940)

### 4.3 描述符账：交出指针不等于交出 buffer 的回收权

顺序是：准备描述符 → 设置 callback 与 callback_param → submit → 保存 cookie → 不再碰提交的描述符 → issue_pending。

submit 后描述符归 DMAengine；cookie 是追踪标识，不能拿 cookie 重新构造一个可访问描述符指针。`dmaengine_submit()` 本身不启动传输，也不保证客户端 buffer 已可以释放。已经排队的请求以后仍可能被 issue，所以“尚未敲启动”不是可以直接 unmap 的理由。[v6.6 guide：提交与启动](https://www.kernel.org/doc/html/v6.6/driver-api/dmaengine/client.html)

v6.6 cookie 是正数请求标识或负数错误，`dma_submit_error()` 检查负数。遇到 submit 错误，客户端不能继续 issue；本章采用保守策略：不复用原描述符指针，先请求成功的 channel 停止/同步，再回收客户端映射。**这项保守策略不是在断言所有 provider 都会执行失败提交**；真实驱动还需核对所用 provider 的失败行为和描述符回收约定。[v6.6 cookie 定义](https://github.com/torvalds/linux/blob/v6.6/include/linux/dmaengine.h#L15-L26)

成功 prep 后，部分 provider 可能在 prep 与 submit 之间持有锁，因此应紧密配对。本章不允许 remove 插在这两步之间，也不提供“任意丢弃未 submit 描述符”的假通用 API。可行的真实设计是让请求入口和移除门禁共用合适的串行化机制，prep 失败就释放该短临界段；prep 成功就完成设置回调并 submit，然后退出。[v6.6 client.rst：prepare/submit](https://github.com/torvalds/linux/blob/v6.6/Documentation/driver-api/dmaengine/client.rst)

### 4.4 访问者账：至少同时保留这些东西

数据 buffer 活着还不够。回调可能访问 request、completion、驱动私有对象、统计字段或后续工作项；相应对象和执行代码必须满足各自的生命周期。

本章模型把 buffer、request 与 callback storage 合并为一个待释放资源包，强制正常路径在 callback_return 后才 unmap/free。这是便于审查的**本章策略**，不是声称所有驱动都必须让数据映射等到回调返回：真实驱动可以设计更细粒度的访问权，但必须独立证明被释放的那个对象没有后续访问者。

如果回调把工作排进 workqueue，DMAengine 的 callback 同步只覆盖 DMAengine 回调，不自动证明该工作项、timer、外设 IRQ handler、用户打开的 fd 或其他引用已经结束。本章不模拟这些扩展访问者；真正驱动必须在释放前逐一断开并同步。

## 5. 超时：结束等待，尚未结束设备访问

### 5.1 为什么 timeout 后直接 unmap 会出事？

原创故障场景：RX 请求映射完成，设备开始写入。用户线程等待 50 ms 后超时，于是走公共 `out_free`。设备在第 51 ms 写入最后一段；原 buffer 已成为别的对象，迟到的 DMA 写损坏了新对象。IOMMU 环境可能报 fault，也可能因为映射/资源的重用而形成更隐蔽的故障。实际后果取决于平台，不能把“测试机没有报错”当作生命周期成立。

正确地问：超时后谁发出取消？谁检查停止返回？谁等待 DMA 和回调静默？谁唯一负责 unmap/free？在获得这些证据之前，超时不能授予释放权。

### 5.2 超时和完成相遇，终态只能选择一次

模型把 `Outcome` 与 `Phase` 分开：

- `timeout()` 将请求结果选择为 TIMEOUT，但 phase 仍是 ACTIVE/QUEUED 等原状态。
- 后来的硬件完成与回调可以结束访问者生命周期，不把已决定的 TIMEOUT 改成 SUCCESS。
- 成功 unmap/free 代表回收安全，不代表这笔接收成功。
- 即使 buffer 已安全回到 CPU，`consume_rx()` 也拒绝 TIMEOUT、CANCELLED、IO_ERROR。

实际并发驱动需要锁或原子状态转换来决定“完成”和“取消”谁赢，以及是否允许回调再发布终态。模型只是串行给出两种先后次序，没有证明真实竞争被正确同步。

### 5.3 completion 通知不能随意代替 callback 静默

`callback_publish` 可以唤醒等待线程，随后 callback 仍未返回。如果等待线程立刻释放 callback_param，而回调还访问这个对象，就是软件侧的 use-after-free。

本章把 publish 与 return 分成两个事件，故意让测试在两者之间尝试 free 并失败。在实际设计里，可以靠明确的引用计数/访问协议或适用的停止同步完成生命周期证明；不能只因为函数里调用了 `complete()` 就忽略它之后的代码和已排出的异步工作。

## 6. 取消的两条路，以及成功返回的意义

### 6.1 可睡眠上下文：成功的同步终止

`dmaengine_terminate_sync(chan)` 在适用的非原子上下文使用；不能在同一通道的完成回调内调用。只有返回成功后，才采用其“传输已停止、运行中的完成回调已结束”的合约作为清理依据。

特别重要：v6.6 wrapper 先调用异步 terminate；若返回错误就立即返回，不调用 synchronize。因此不能只看调用过 `terminate_sync` 这个事实，必须检查返回值。[v6.6 terminate_sync 实现](https://github.com/torvalds/linux/blob/v6.6/include/linux/dmaengine.h#L1105-L1129)

“同步调用”也不保证有一个普遍适用的固定完成时限。本章不模拟卡死的 provider 或永不返回的调用；真实系统的 watchdog、恢复和超时策略要依目标驱动设计，不能在等待期限到达后又偷换成“安全 free”。

### 6.2 原子上下文/回调：异步停止，再在进程上下文同步

步骤是：

1. 关闭相应新工作入口，避免停止过程中又加入新的请求。
2. 调用适用的 `dmaengine_terminate_async()`，检查返回值。
3. 返回成功也只表示接受停止；保留 buffer 映射和所有回调可达资源。
4. 安排非原子、非该通道回调上下文调用 `dmaengine_synchronize()`。
5. synchronize 返回后再回收；从 async terminate 到 synchronize 之间不调用 issue_pending。

`synchronize` 自身返回 void，本章的事件表示它**已经等待结束并返回**，不是刚进入等待。没有成功的 async terminate，不能只调用 synchronize 就制造停止成功的依据。[v6.6 synchronize 合约](https://github.com/torvalds/linux/blob/v6.6/include/linux/dmaengine.h#L1081-L1104)

### 6.3 不完整的取消不会自动触发完成回调

终止可能丢弃尚未搬完的 FIFO 数据；不完整传输不会被调用完成回调。因此“取消后等那次 completion 回调来释放 request”可能永远等不到。清理负责人需要主动为请求选择取消/超时终态，并在停止同步后唤醒必要的等待者、回收资源。

已经完成并待调用、或已经进入的回调是另一种竞争；不能把“取消的未完成请求不会新调用回调”曲解为“调用 terminate_async 后绝不再看见回调”。[v6.6 DMAengine termination guide](https://www.kernel.org/doc/html/v6.6/driver-api/dmaengine/client.html#further-apis)

### 6.4 常见的假替代

- `dma_sync_*_for_cpu()` 是适用的访问权/cache 同步操作，不是停止 DMA 的命令。
- 关闭一个外设 IRQ 不等于 DMA 控制器停止，也不等于 tasklet 回调结束。
- `DMA_PAUSED` 不是所有资源已失去访问者；paused 不等于 terminal。
- 保存的 cookie、轮询到一个状态或 residue 数字，不会自动取消其他软件访问者。具体状态查询能力依 provider 核对。
- coherent allocation 也不能在设备或回调仍使用时 free；cache 一致性不消除生命周期问题。

## 7. remove：先关门，再处理既有请求

### 7.1 建议的审查顺序

这是设计检查表，不是可直接复制的某个总线 `.remove()` 实现：

1. **关闭新提交入口**：在与请求进入/提交相匹配的同步机制下标记移除状态。
2. **停止其他生产者**：按驱动架构禁止新的外设请求、timer/workqueue 重提、用户请求等；逐一核对，不把一个 bool 当作万能门禁。
3. **固定在途请求集**：让已经进入提交临界段的操作退出，确保后续停止覆盖所有已提交请求。本章只有一笔，不存在队列快照遗漏。
4. **避免等待锁死**：退出回调需要的锁/临界段，再执行可睡眠的停止与同步。不能持有 callback 要获取的锁，同时等它结束。
5. **取得 quiescence 证据**：适用的同步停止成功，或成功异步停止之后 synchronize 返回。
6. **归还映射并释放**：每笔成功 map 对应一次参数匹配的 unmap，再释放相应 buffer/request；更细粒度对象先证明各自无人使用。
7. **释放通道与其他资源**：按真实驱动资源依赖顺序释放；本章以所有 request 资源结束后释放独占 channel。
8. **结束移除**：只有所需访问者都静默后，才宣称移除清理完成。

门禁与清理必须同时成立：只同步回调但允许新 submit，会重新产生在途访问；只关闭入口但不处理旧访问，也不能 free。

### 7.2 为什么单个 removing 标志不足？

可能的错误时间线：

1. 提交者读取 removing=false。
2. remove 将 removing=true，发现“没有请求”，释放通道。
3. 提交者使用旧 channel 做 prep/submit。

标志的值没有解决检查和使用之间的竞争。真实设计必须给提交者建立适用的引用和串行化协议。本章中 `begin_remove()` 不允许插入 PREPARED 状态：它表达“prepare/submit 与 remove 共用门禁临界段”的设计选择，而非假设实际 kernel 自动帮客户端串行化。

模型在 map 与 prepare 之间允许关闭门禁，因为此时尚未产生描述符，可以回收已有映射；prepare 失败同理。成功 prep 后不凭空发明一个销毁未提交 descriptor 的通用接口。

## 8. 停止失败：保留与隔离不是“已经安全移除”

若 terminate 返回错误，缺失的证据是“设备与回调不再访问”。失败路径应：

- 关闭新工作的门禁，记录失败与在途 request 信息。
- 保留可能被访问的映射、buffer、request、callback storage 及必要的对象/代码生命周期。
- 不重复 unmap，不 release channel，不把 request 标为已安全释放。
- 交给该控制器/总线真正支持的恢复策略，只有之后取得可靠的停止证据才继续清理。

本章称这为 **QUARANTINE**，指软件不再重用这些资源。它不是声称硬件被隔离，也不是一个已经实现的安全机制。buffer 不 free 但 driver private object 或 callback 代码已卸载，同样危险；单纯“漏掉一块内存”不是完整策略。

也不能假设所有 Linux `.remove()` 都能返回错误并拒绝移除。不同总线、驱动接口版本、设备真正消失时的行为都不同。本章 `finish_remove()` 拒绝失败路径，是主机模型的检查规则；不是声称内核移除框架支持这种回滚。实际驱动必须提前设计硬件停机能力、对象引用、解除绑定/卸载和失败收容路径。若无法保证这些，就必须明确这是未解决的风险，而非把代码写成 unconditional free。

模型允许“稍后再次停止并确认成功”后清理，但不模拟 reset、不认为 IOMMU fault 就等于安全，也不把资源保留称作生产级恢复验证。

## 9. 五条手推轨迹

### A. 正常 RX：3 项 SG 合并成 2 段

| 步骤 | 事件 | 保留/允许的访问 |
|---|---|---|
| 1 | CPU 分配并准备 request | buffer 与 callback storage 活着 |
| 2 | map(3) 返回 2 | 记录原始 3、返回 2；CPU 停止 payload 访问 |
| 3 | prep(2)、设置回调、submit | 描述符指针交出；映射与 request 保留 |
| 4 | issue_pending | 设备可能访问 buffer |
| 5 | hardware_complete | 搬运完成，回调仍 pending |
| 6 | callback_enter、publish | 等待者可被唤醒，回调仍 running |
| 7 | callback_return | 本章正常路径的回调静默证据 |
| 8 | unmap(原始 3) | CPU 可以访问；RX 结果另核对成功标签 |
| 9 | free | 资源包不再有访问者 |

第 5、6 步尝试 unmap/free 都被本章模型拒绝；这让“硬件完成”和“软件生命周期结束”可见。

### B. 等待超时后异步取消

`ACTIVE → timeout → STOPPING → synchronize 返回 → QUIESCED → unmap → free`

TIMEOUT 不把 ACTIVE 改成 DONE。STOPPING 中 issue_pending 被拒绝，unmap/free 被拒绝；最终安全回收后结果仍是 TIMEOUT，不能作为成功数据交给上层。

### C. 回调已发布通知，remove 同时到来

`HW_DONE → callback_enter → callback_publish → close_gate → terminate_sync 成功 → unmap → free → release_channel`

发布通知之后，callback_running 仍为真；单独 free 会被拒绝。同步停止事件表示等待结束，才允许释放 callback_param 所在的资源包。本章不会通过真实线程调度来证明这条竞争。

### D. 映射与准备失败

- map 返回 0：`CPU → MAP_FAIL → close_gate → free → release_channel`，map/unmap 账是 0/0。
- map 成功、prep 返回 NULL：`MAPPED → PREP_FAIL → close_gate → unmap → free`，账是 1/1。
- submit 返回负 cookie：`SUBMIT_FAILED → terminate_sync 成功 → unmap → free`；不再 issue，不复用提交时交出的指针。provider 特有的描述符错误回收需另查。

区别在于实际已经取得的资源，不使用同一条无条件 `out_unmap` 处理所有失败。

### E. remove 时停止失败

`close_gate → terminate 错误 → QUARANTINE`

此时 buffer、映射与回调存储仍活着，通道未释放；模型拒绝 unmap/free/finish_remove。后来可靠地停止成功，才走 `QUIESCED → unmap → free → release_channel → finish_remove`。若可靠停止始终不存在，这条轨迹就保持未完成。

## 10. 可执行主机状态实验

文件：

- [状态模型](../../examples/python/dma_lifecycle_contract.py)
- [聚焦测试](../../tests/test_dma_lifecycle_contract.py)

从仓库根目录运行：

```bash
PYTHONDONTWRITEBYTECODE=1 python3 examples/python/dma_lifecycle_contract.py
PYTHONDONTWRITEBYTECODE=1 python3 tests/test_dma_lifecycle_contract.py
```

只依赖 Python 标准库。本次使用 Python 3.12.14；没有访问设备节点、真实地址、DMA 控制器、内核接口或网络。实验文件用 `ContractError` 拒绝非法步骤；对象不可变，拒绝操作不会半途改变旧状态。

### 10.1 模型变量为什么这样拆？

- `Phase`：请求所在生命周期阶段。
- `Outcome`：调用者看到的终态，独立于阶段。
- `map_count/unmap_count`：成功映射的配对账，单请求最大 1/1。
- `descriptor_client_owned`：只在 PREPARED 为真，submit 后为假。
- `dma_may_access`：保守地标识映射必须保留的设备侧风险；QUEUED 也保留，因为之后可能启动。
- `callback_pending/callback_running`：拆开尚未调用与正在执行。
- `removal_requested/quarantined`：关闭新工作的不同原因。
- `channel_released/removed`：最终状态必须依赖前面全部清理。

`dma_may_access` 不是读真实硬件寄存器得来的状态。特别是失败提交时设为真，是本章保守的清理策略，而不是测得设备已经启动。

### 10.2 一段可修改的实验

以下 Python 片段需把 `examples/python` 加入导入路径，或在同目录交互执行：

```python
from dma_lifecycle_contract import Lifecycle, ContractError

s = Lifecycle().map_sg(2).prepare().submit().issue_pending()
s = s.timeout().begin_remove().terminate_async()

try:
    s.unmap_sg()                 # 必须失败：停止尚未同步
except ContractError as error:
    print(error)

s = s.synchronize().unmap_sg().free_resources()
s = s.release_channel().finish_remove()
assert s.removed
assert s.outcome.value == "timeout"
```

代码是主机对象事件，不是内核 API 封装。删除 `synchronize()`，测试应拒绝 unmap；删掉 `begin_remove()`，最终 release_channel 应被拒绝。尝试 `original_nents=2` 去解除默认原始 3 项的映射，也必须失败。

### 10.3 本次聚焦测试的实测范围

28 项 unittest 通过，其中正常 SG 分支覆盖原始项数 1～4、所有返回段数 1～原始项数，以及 RX/TX 两个方向，共 20 条正常路径。其他测试包含：

- map 失败、prep 失败、负 cookie 提交错误。
- controller device、SG 数量、方向参数不一致。
- CPU 提前访问、重复 map/unmap/free/release/remove。
- timeout、迟到成功回调、不完整取消没有回调。
- hardware_done 但 callback pending；publish 后 callback 仍 running。
- 已排队但未 issue 的请求在 remove 时正确取消。
- async stop 与已排队/正在运行回调相遇。
- atomic/callback 上下文拒绝同步停止与 synchronize。
- 停止失败保留全部资源，之后验证成功才恢复清理。
- prepare/submit 对 remove 的设计串行化边界。

另有深度至多 14 个事件的去重状态图遍历：**295 个抽象状态、5,701 条被拒绝边、15 个完成移除状态**。这只是所选事件集合与深度的有限检查；未遍历所有长度、所有 provider、所有线程交错，也不是形式化证明。事件允许显式表达 synchronize 的已返回结果，不能检测一个真实 provider 是否遵守合约。

## 11. 面试追问链：每问再往下一层

### 链 1：地址修好了，为什么仍旧数据/崩溃？

1. 用了 `dma_addr_t`，是不是就对？还要看映射失败、正确 DMA device、方向与访问权窗口。
2. RX 完成后 CPU 直接读？本章短映射先 unmap；长映射策略需在正确静止窗口做适用 sync。
3. 加 sync 能解决 remove 崩溃？sync 不停止设备，也不排空 callback。
4. 换 coherent 能解决？coherent 仍要有访问权、顺序与对象生命周期设计。
5. x86 正常、非一致性 Arm 出错说明什么？只能作为查 cache/映射规则的线索，不能替代目标平台证据。

### 链 2：超时为何不能走普通释放路径？

1. wait 超时证明谁停止？只证明这个等待条件没及时满足。
2. 取消会自动有回调吗？未完成传输被终止时不能依赖完成回调。
3. async terminate 返回成功，能 free 吗？还缺 synchronize 返回。
4. synchronize 和 issue_pending 可以交错吗？v6.6 明确给出未定义行为边界，应阻止新启动。
5. 超时后迟到的成功怎么处理？终态选择与生命周期清理分开；需要真实同步，禁止双重完成/双重 free。

### 链 3：remove 和回调分别要什么锁？

1. removing=true 就足够？不够，要消除检查后使用竞争。
2. 持锁 terminate_sync 更保险？若 callback 要同一把锁，可能死锁；先看等待依赖图。
3. callback 里 terminate_sync？不允许该同步上下文；可用适用 async 方案并移交同步清理。
4. callback 里排 work，sync 后 work 安全？不自动成立，work 的引用与取消/drain 单独审查。
5. 停止失败呢？保留资源并报告未完成，不能无限扩大“成功清理”的含义。

### 链 4：为什么同时保存两个 SG 数？

1. map 返回段数小于原始项数可能吗？可以，映射可合并。
2. prep 用哪个？返回 DMA 段数。
3. unmap/SG sync 用哪个？原始输入项数，并保持 device/direction 匹配。
4. map 返回 0，也 unmap？没有本次成功映射，不能伪造配对账。
5. SG 重复 map 可以吗？已映射 SG 不重复 map；映射过程允许改变 SG 信息。

## 12. 常见坑与验收清单

### 红旗写法

- `dma_unmap_sg(..., dma_nents, ...)`，误把合并后的段数当原始项数。
- 用外设 `dev` 映射，用 controller `dev` 同步/解除，或反过来。
- `wait_timeout` 失败后无条件 `kfree`。
- `terminate_async` 后直接 unmap，或中间再次 issue_pending。
- 忽略 terminate 返回值，日志却写“设备已经停止”。
- 回调刚 publish 就释放 callback_param，未证明尾部代码/其他异步使用者结束。
- remove 门禁只是未同步的普通 bool，提交者拿旧引用继续操作。
- cancellation path 等待一个不会到来的完成回调。
- descriptor submit 后继续改 callback 字段。
- 只保留 payload，释放仍会被回调访问的驱动对象或代码。

### 从教学模型到目标驱动前要补的证据

- 确认具体内核版本、DMA controller/provider 和总线接口。
- 确认可映射内存、正确 mapping device、寻址能力、段数/长度限制与方向。
- 记录所有错误分支的资源账，清理负责人唯一且终态只发布一次。
- 确认门禁覆盖请求入口、prepare/submit/issue、callback 重提及工作队列生产者。
- 画 callback 与 remove 的锁依赖图，避免持回调依赖锁等待。
- 确认成功停止/同步的真实语义，不能以 timeout/pause/cache sync 代替。
- 对真正外设另查 FIFO、帧完整性、外设 IRQ、reset 与热拔除策略。
- 在隔离内核/硬件测试环境验证映射错误、取消、卸载、迟到回调和停止失败；本章未执行这些验证。

本章的完成标准：能给出一条正常轨迹、三条失败轨迹；每次回收都说清“最后一个访问者是谁，结束证据是什么”；聚焦 host 测试通过且没有把结果扩大成内核、cache、并发或板级正确性结论。

## 13. 先修、横向联系与来源

- [DV05：地址与 streaming/coherent 基础](dv05.md)
- [IC04：DMA 故障场景复习](../120-interviews/ic04.md)
- [MC06：Cortex-M7 cache 与 DMA 的平台边界](../40-mcu/mc06.md)
- [可选深化：发布顺序与所有权合约](../10-c-cpp/publication-ownership-contracts.md)：CPU 间发布顺序不自动替代 DMA API；生命周期需要单独证明。
- [可选深化：NVIC 优先级审查](../50-rtos/nvic-priority-audit.md)：上下文约束也不能跨 RTOS/Linux 直接套用。

一手来源均固定至 Linux v6.6，正文、模型、测试与故障场景为本仓库原创：

1. [DMA API HOWTO](https://www.kernel.org/doc/html/v6.6/core-api/dma-api-howto.html)：地址、可映射内存、streaming 访问权和错误处理。
2. [DMA API reference](https://www.kernel.org/doc/html/v6.6/core-api/dma-api.html)：SG 返回值、原始项数和配对参数。
3. [Slave DMAengine client guide](https://www.kernel.org/doc/html/v6.6/driver-api/dmaengine/client.html)：mapping device、prep/submit、启动、取消与 callback 资源。
4. [固定版 client.rst 源文件](https://github.com/torvalds/linux/blob/v6.6/Documentation/driver-api/dmaengine/client.rst)：与网页交叉核对。
5. [固定版 dmaengine.h](https://github.com/torvalds/linux/blob/v6.6/include/linux/dmaengine.h)：实际类型、cookie、helper 与同步终止的错误分支。
6. [固定版 dma-api.rst](https://github.com/torvalds/linux/blob/v6.6/Documentation/core-api/dma-api.rst)：SG map/unmap/sync 的精确参数合约。

来源用于核对 API，不代表来源作者为本仓库模型背书。主机模型采用更严格的一次性策略，不能用于证明任意真实驱动已符合这些合约。
