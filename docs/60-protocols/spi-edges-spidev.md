# SPI 四模式逐边沿与 spidev 命令数据事务

学习优先级：P2，可选补充。先读 [PR02 模式与片选](pr02.md) 和 [LX12 Linux 子系统接口](../70-linux-apps/lx12.md)。本页从“知道 CPOL/CPHA”推进到“能解释第一个与最后一个边沿、把完整事务交给接口、验证每条失败路径”，不新增核心题号，也不重写 SPI 入门或器件驱动。

软件语义固定为 Linux **v6.6** 的 spidev UAPI、用户文档及有关驱动分支。下述逐边沿图表、合成设备合同、C 代码与 mock 测试为原创；没有运行真实 `/dev/spidev*`、内核 SPI 控制器、目标器件或电气实验。具体芯片的命令、地址、dummy 周期和时序应由其数据手册另行规定，本页不推测任何芯片寄存器。

## 直接回答

先把 SCK 从空闲电平离开的沿叫 leading edge，把返回空闲电平的沿叫 trailing edge。CPOL 选择空闲电平；CPHA=0 在 leading 采样，CPHA=1 在 trailing 采样。因此采样物理沿依次为 Mode 0 上升、Mode 1 下降、Mode 2 下降、Mode 3 上升。[Linux v6.6 spidev 模式说明](https://www.kernel.org/doc/html/v6.6/spi/spidev.html#basic-character-device-api)

命令与数据需要同一 CS 窗口时，使用一条 `SPI_IOC_MESSAGE(2)`，包含命令和数据两个 `spi_ioc_transfer`，两段的 `cs_change` 都为 0。每个时钟同时移出 MOSI 位和移入 MISO 位；所谓“读取”仍要产生时钟，数据段可以一边发送 dummy 字节、一边接收应答。配置成功、消息返回预期字节数和器件接受命令是三个不同结论。

## 原理拆解

### 一 先定义边沿再换成上升下降

本节假设普通独立 MOSI/MISO、8 位字、MSB-first。`b7` 是当前字节首先传的位；CS 的有效极性不由 CPOL 决定。为了画图可假设 CS 低有效，但把表用于硬件前必须核对实际片选极性。

| 模式 | CPOL | CPHA | SCK 空闲 | leading | trailing | 首位准备 | 采样沿 |
|---|---:|---:|---|---|---|---|---|
| 0 | 0 | 0 | 低 | 上升 | 下降 | 第一个上升沿之前已稳定 | 上升 |
| 1 | 0 | 1 | 低 | 上升 | 下降 | 第一个上升沿启动首位输出 | 下降 |
| 2 | 1 | 0 | 高 | 下降 | 上升 | 第一个下降沿之前已稳定 | 下降 |
| 3 | 1 | 1 | 高 | 下降 | 上升 | 第一个下降沿启动首位输出 | 上升 |

“输出/改变沿”表示发送器在这一沿附近启动下一位的输出过程，**不表示引脚在零延迟瞬间变为新值**。接收器要求数据在采样沿前后满足 setup/hold；真实输出延迟、走线延迟和 SCK 偏斜仍要列入时序预算。

CPHA=0 的首位不能等第一个采样沿才发送：CS 有效后，发送器要提前准备 `b7`，并满足片选到首个时钟沿、首位数据到采样沿的约束。CPHA=1 的第一个 leading 沿启动 `b7`，随后 trailing 才采样它。两种模式都不能把“CS 刚有效”与“数据立刻有效”当作零时间事件。

### 二 一个字节的全部十六个边沿

从 SCK 空闲电平开始计数 E1…E16。表内“采”是接收器采样当前位，“出”是发送器启动该位输出；MOSI 与 MISO 各自遵循双方约定的相位。表是理想数字教学模型，不带实际纳秒参数。

| 边沿 | Mode 0 | Mode 1 | Mode 2 | Mode 3 |
|---|---|---|---|---|
| E0，时钟尚未开始 | 先稳定 b7 | 等首沿 | 先稳定 b7 | 等首沿 |
| E1 | ↑ 采 b7 | ↑ 出 b7 | ↓ 采 b7 | ↓ 出 b7 |
| E2 | ↓ 出 b6 | ↓ 采 b7 | ↑ 出 b6 | ↑ 采 b7 |
| E3 | ↑ 采 b6 | ↑ 出 b6 | ↓ 采 b6 | ↓ 出 b6 |
| E4 | ↓ 出 b5 | ↓ 采 b6 | ↑ 出 b5 | ↑ 采 b6 |
| E5 | ↑ 采 b5 | ↑ 出 b5 | ↓ 采 b5 | ↓ 出 b5 |
| E6 | ↓ 出 b4 | ↓ 采 b5 | ↑ 出 b4 | ↑ 采 b5 |
| E7 | ↑ 采 b4 | ↑ 出 b4 | ↓ 采 b4 | ↓ 出 b4 |
| E8 | ↓ 出 b3 | ↓ 采 b4 | ↑ 出 b3 | ↑ 采 b4 |
| E9 | ↑ 采 b3 | ↑ 出 b3 | ↓ 采 b3 | ↓ 出 b3 |
| E10 | ↓ 出 b2 | ↓ 采 b3 | ↑ 出 b2 | ↑ 采 b3 |
| E11 | ↑ 采 b2 | ↑ 出 b2 | ↓ 采 b2 | ↓ 出 b2 |
| E12 | ↓ 出 b1 | ↓ 采 b2 | ↑ 出 b1 | ↑ 采 b2 |
| E13 | ↑ 采 b1 | ↑ 出 b1 | ↓ 采 b1 | ↓ 出 b1 |
| E14 | ↓ 出 b0 | ↓ 采 b1 | ↑ 出 b0 | ↑ 采 b1 |
| E15 | ↑ 采 b0 | ↑ 出 b0 | ↓ 采 b0 | ↓ 出 b0 |
| E16 | ↓ 返回空闲 | ↓ 采 b0，返回空闲 | ↑ 返回空闲 | ↑ 采 b0，返回空闲 |

CPHA=0 在 E15 已采到末位，但正常完整时钟周期还包括 E16；不能因此删掉最后半个周期或提前撤销 CS。若后面紧接下一字节，E16 附近还可启动下一字节 `b7`，为下一次 E1 做准备，同时仍须满足上一位 hold。CPHA=1 则到 E16 才采到末位，之后也不能无视采样后保持时间就释放片选。

以 `0xA6 = 10100110` 为手推输入，四种模式均应恢复 `1,0,1,0,0,1,1,0`：

- Mode 0/2：在 E1、E3、E5、E7、E9、E11、E13、E15 采样
- Mode 1/3：在 E2、E4、E6、E8、E10、E12、E14、E16 采样
- 把 Mode 0 的解码器设为 Mode 1，不会自动证明“总会左移一位”；采样落在改变附近时，结果还依赖实际延迟、首尾位和分析仪算法

8 位的 MSB/LSB-first 是另一维度。改变位序不会改变采样边沿；把 Mode 0 换成 Mode 3 也不能当作位序转换。字长不是帧长，CS 窗口可以包含多个字。

### 三 时序预算不能只剩一个频率

先记目标器件要求的 SCK 高/低时间、数据 setup/hold、输出有效延迟、CS 到首沿、末沿到 CS 撤销，以及 CS 无效最短间隔。再检查控制器怎样表达和落实这些要求。理想 50% 占空比下，相邻输出沿与采样沿约相隔半个周期；那只是可分配预算，不能忽略输出延迟、线路负载、板级偏斜和接收输入约束。

把 SCK 降低有助于某些 setup 问题，但不保证修复极性错误、CS 接错、MISO 争用、电压不匹配或 hold 问题。逻辑分析仪可帮助数位和查帧；边沿质量、阈值和模拟时序裕量需要示波器及适当探头。这里没有给任意芯片推荐频率或电压。

## 例子与推演

### 四 先写一个不冒充芯片的完整合同

本页合成从机只用于 mock：

1. 一个事务有 2 个不解释语义的命令字节 `0x80, 0x81`
2. 从机在紧随其后的 4 个字节期间返回合成应答；不要求额外等待周期
3. 数据阶段 MOSI 固定送 `0xFF`，MISO 同时接收 4 字节
4. 命令到数据之间 CS 保持有效；事务结束后按常规消息边界撤销
5. 配置例为 Mode 0、MSB-first、8 bits-per-word、请求最大 1 MHz

这不代表某颗传感器的命令或寄存器。测试用确定性函数产生假应答，不能把输出拿去与真实芯片 ID 比较。

理想传输是 `2 + 4 = 6` 个字节，共 48 个 SCK 周期、96 次电平转换。在假设实际 SCK 恰好 1 MHz、没有任何间隙时，移位部分为 48 μs；CS setup/hold、transfer 间停顿、驱动排队和调度时间另算。示例没有测得 48 μs，也没有建立 Linux 实时完成上界。

| 段 | MOSI | MISO | len | tx_buf | rx_buf | cs_change |
|---|---|---|---:|---|---|---:|
| transfer[0] | 2 字节命令 | 输入被丢弃 | 2 | command | 0 | 0 |
| transfer[1] | 4 字节 0xFF | 合成应答 | 4 | dummy | 私有 received | 0 |

命令阶段 `rx_buf=0` 只表示软件不保留移入数据，不表示 MISO 没有位活动。数据阶段同时设置 `tx_buf` 和 `rx_buf`，因此是一个真正表达全双工缓冲的 transfer，而不是先写 dummy 再额外读一次。`len=4` 是共同的移位字节数，不是 TX 4 再 RX 4 共计 8。[Linux v6.6 spidev 全双工说明](https://www.kernel.org/doc/html/v6.6/spi/spidev.html#full-duplex-character-device-api)

`tx_buf=0` 的接口语义是移出零，本示例显式提供 dummy 以展示所需值；`0xFF` 并非 SPI 的通用读常量。还要区分“为读数据而持续送出的无关字节”和“器件要求的额外等待时钟”：如果真实器件规定命令后先空转 8 clocks、随后才输出有效数据，就要明确增加一个 dummy 字节并丢弃相应接收内容，或另列等待段。本示例的 4 字节 reply 没包含这种等待，不允许悄悄把第一字节错当有效数据。[Linux v6.6 spidev UAPI 字段合同](https://raw.githubusercontent.com/torvalds/linux/v6.6/include/uapi/linux/spi/spidev.h)

### 五 一条 message 的 CS 边界

期望顺序是：CS 有效 → 命令 transfer → CS 仍有效 → 数据 transfer → 消息结束，CS 撤销。`SPI_IOC_MESSAGE(2)` 的数字 2 是 **transfer 数量**，不是字节数，也不是数组字节大小。数组有两项就提交两项；别在宏里塞 `sizeof transfers`。

两次独立 `SPI_IOC_MESSAGE(1)` 或 `write()` 再 `read()` 是两次消息/操作边界，不能用“中间没有主动写 GPIO”证明 CS 连续。普通 read/write 之间会撤销片选。[Linux v6.6 基本字符设备 API](https://www.kernel.org/doc/html/v6.6/spi/spidev.html#basic-character-device-api)

`cs_change` 也不是“最后请释放片选”的统一布尔开关：

- 非末 transfer 设为 1：请求该段之后让 CS 暂时无效，再进入下一段，可能把命令与应答拆成不同帧
- 末 transfer 设为 1：有不同语义，可能让器件在消息后保持选中；不能据此取得跨任意下一条消息的独占保证
- 本页两段都为 0：采用普通连续 message，不请求中途切换或结束后延续

一条 message 的传输序列不会与同一 SPI 总线的其他 message 交错；这不等于整个“多个配置 ioctl + message”是单一不可分割业务事务。另一个进程仍可能在调用间改变共享设置。`CS 连续` 也不保证 `SCK 连续无停顿`，控制器可能在两段间有间隙。[Linux v6.6 spi_transfer / spi_message](https://www.kernel.org/doc/html/v6.6/driver-api/spi.html#c.spi_transfer)

本实验的调用者必须拥有整次配置与事务期间的设备使用权。一个设备拥有者线程/进程可以串行接受业务请求；同一进程内的 mutex 不能自动阻止其他进程。如果协议必须无间隙、在 CS 内根据新收到的数据动态计算下一段，或者要求接口不能表达的极短时序，应重新评估驱动和控制器能力，不能靠 userspace `sleep()` 冒充硬件时序控制。

### 六 可编译 C 实现的配置与内存责任

完整文件是 [接口](../../examples/c/spidev_transaction.h)、[实现](../../examples/c/spidev_transaction.c) 与 [完整 mock 调用方及测试](../../tests/host_spidev_transaction_test.c)。只有注入回调，没有真实 syscall adapter、`open()` 或真实设备入口。测试 fd=37 只是传给 mock 的标记。

配置调用逐项检查，任何一步失败都不提交消息：

| 次序 | 调用 | 参数与检查 |
|---:|---|---|
| 1 | SPI_IOC_WR_MODE32 | uint32_t mode，返回必须为 0 |
| 2 | SPI_IOC_RD_MODE32 | uint32_t actual_mode，返回 0 且等于要求 |
| 3 | SPI_IOC_WR_BITS_PER_WORD | uint8_t word=8，返回 0 |
| 4 | SPI_IOC_RD_BITS_PER_WORD | uint8_t actual_word，返回 0 且为 8 |
| 5 | SPI_IOC_WR_MAX_SPEED_HZ | uint32_t speed，非零，返回 0 |
| 6 | SPI_IOC_RD_MAX_SPEED_HZ | uint32_t actual_speed，返回 0 且等于请求值 |
| 7 | SPI_IOC_MESSAGE(2) | 两段数组，返回必须为 command_len+reply_len |

本接口仅接受 mode=0…3，不保留 LSB-first、3-wire、NO_CS、双/四/八 lane 等扩展模式。写 MODE32 请求普通 MSB-first 模式，并读回核对；需要其他模式应改合同和测试。示例刻意只支持 8 位字；官方接口中 word=0 有 8 位含义，这里输入/回读采用显式 8 的更窄合同。更长 word 的内存表示、部分字和字节序不在本例支持范围内。[Linux v6.6 配置 ioctl](https://www.kernel.org/doc/html/v6.6/spi/spidev.html#basic-character-device-api)

speed 的读回只核对配置值。控制器不一定能产生恰好所请求的速率；spidev 用户接口不能据此提供实际 SCK 的测量值。transfer 内再次显式填 `speed_hz` 与 `bits_per_word` 是本例的逐段合同，不把默认值当已知；这些字段也不能阻止其他进程改变 mode。

实现的关键责任是：

- 用 `memset(transfers, 0, sizeof transfers)` 清零整个数组，包含未使用字段与 pad；命令段不填 rx，所有 delay/lane 字段维持零
- UAPI 指针字段是 `__u64`，先把用户指针转换为 `uintptr_t` 再转为 `__u64`；它仍是用户虚拟地址，不是物理地址或 DMA 地址
- command 最大 16 字节、reply 最大 64 字节；在长度求和和窄化转换前拒绝零长、超界与空指针，预期总长度最多 80
- dummy 和 received 为函数栈上的有界缓冲；同步回调必须在返回前用完这些指针，不能缓存后异步访问
- 外部 reply 只在消息完整成功后由私有 received 复制，失败不向调用者发布部分数据；report 应放在独立对象中
- `errno` 只在回调返回 -1 后保存；成功后的旧 errno 没有诊断含义

本实验不保存/恢复设备原配置，也不宣称配置失败没有副作用。例如 mode 写成功、word 写失败时，mode 已可能改变。下一位设备拥有者必须显式配置；多步恢复也可能失败且不具备设备侧回滚能力。[Linux v6.6 spidev.c 的配置与消息分支](https://github.com/torvalds/linux/blob/v6.6/drivers/spi/spidev.c)

### 七 返回值与失败后的业务状态

对合成 2+4 字节消息，正常成功返回是 **6**。不是 2 个 transfer，不是 4 字节 reply，也不是 2+4+4=10。Linux v6.6 `spidev_message()` 在成功同步和接收复制之后返回各 transfer 的 `len` 总和；本例据此检查准确计数。驱动返回的成功计数不是设备侧 ACK。[Linux v6.6 spidev_message](https://github.com/torvalds/linux/blob/v6.6/drivers/spi/spidev.c)

| 结果 | 本例分类 | 调用方 reply | 下一步含义 |
|---|---|---|---|
| 配置 -1 | CONFIG_IOCTL | 不变 | 记录具体配置阶段与 errno，消息未提交 |
| 配置异常非零 | CONFIG_RETURN | 不变 | 拒绝不符合本回调合同的结果 |
| 回读值不匹配 | CONFIG_MISMATCH | 不变 | 配置无法满足要求，消息未提交 |
| message -1 | MESSAGE_IOCTL | 不变 | 记录 errno；设备操作结果可能不确定 |
| message 非预期计数 | MESSAGE_LENGTH | 不变 | 整体拒绝；不把它转换为流式补发 |
| message=预期总长 | OK | 一次性发布私有接收值 | 只表示本接口完成，仍需解释应答 |

异常长度在 mock 中是主动注入的防御路径，**不是声称 Linux v6.6 正常会把 message 当串口一样短写**。mock 还注入 -2，验证违反 libc 返回约定的值也不会误判成功；正常系统调用错误是 -1/errno。

message 失败时不自动重发，也不自动把“剩余字节”追加进下一条 message。CS 窗口已经可能结束，先前命令还可能执行了。甚至数据传输成功、之后复制接收数据失败，也可表现为错误。本例不对 `EINTR` 盲目循环重试；要恢复，需器件规定的事务取消/状态查询、可重复命令或业务幂等依据。

同步接口不等于带 timeout。这里没有给 message 加用户态 deadline，也没有证明杀线程或关闭 fd 可安全取消正在执行的设备命令。需要完成时限时，另查目标控制器/驱动的超时、取消和恢复合同。SPI 没有通用低层 ACK；未连接从机也可能获得传输成功和固定/漂浮输入。验证芯片存在或命令完成必须解释设备协议返回。

## 常见误区与失败边界

1. “0/3 都是上升沿采样，所以互换没区别。”空闲电平与首位启动不同，CS 后的首个沿不能忽略。
2. “每段都是一个 transfer，因此每段 CS 必须跳变。”默认 message 内可以连续片选；`cs_change` 要按位置解释。
3. “把最后段 cs_change=1 就能收尾。”末段非零可能延续选中，不能当普通结束释放。
4. “收 4 字节无需发数据。”读取仍需时钟；显式 dummy 与 NULL TX 的零输出不是任意设备都等价。
5. “配置回读 1 MHz 证明引脚是 1 MHz。”读的是设置；实际时钟与板级裕量未测。
6. “reply 没改说明设备没动作。”私有暂存只保护软件发布，不能撤销命令或配置副作用。
7. “总长很小一定任何主机都接受。”spidev 缓冲上限、按段对齐、控制器长度与能力另有约束；16/64 是课堂上界，不是探测结果。

## 面试追问

- 为什么 CPHA=0 必须提前放首位？因为第一个 leading 已是采样沿，首位要在它之前满足 setup，而不是等到该沿才启动。
- 命令 2 字节、reply 4 字节，为什么不检查返回 4？返回是整个 message 的移位总字节数 6，第二段 TX/RX 同时发生，计一次。
- 两 transfer 能否证明每段无间隙？不能。message 提供组合与片选语义，实际连续时钟和间隙要求需控制器/波形证据。
- 读回 mode 后还会被改变吗？可能。多个 ioctl 的组合没有自动取得跨进程业务独占，须有明确设备拥有者。
- 错误时怎样决定重试？先确定命令是否已执行、是否可幂等重做以及器件恢复入口，不能仅看 errno。

## 动手练习与验收

从仓库根目录编译并运行，仅需 Linux 主机的 spidev 开发头文件与 GCC；运行不需要设备节点或 root：

```bash
gcc -std=c11 -O1 -g -Wall -Wextra -Wpedantic -Werror \
  -Wconversion -Wsign-conversion \
  examples/c/spidev_transaction.c tests/host_spidev_transaction_test.c \
  -o /tmp/embedded-spidev-test
/tmp/embedded-spidev-test
```

2026-10-11 的 x86_64 容器使用 GCC 14.2.0，结果：109 个成功用例、37 个注入错误/异常返回/回读不符用例、16 个非法输入用例、1024 个理想逐边沿字节模型全部通过。

成功矩阵覆盖四种 mode，command 长度 1/2/16、reply 长度 1/4/64、dummy=0x00/0xFF/0xA5；另测不要求 report 的调用。mock 逐项验证配置顺序、32/8 位参数、单条 MESSAGE(2)、两段长度、缓冲内容、全双工第二段及所有零保留字段。所有配置/消息位置分别注入 EIO、EINTR、ETIMEDOUT，要求立刻停止而无重试；另测配置正值、三处回读不符，以及消息 0、少/多于预期、INT_MAX 和 -2。即使 mock 在失败前写入私有 received，外部 reply 也必须维持原值。

理想边沿模型遍历 4 modes × 256 字节，按 leading/trailing 的定义产生/采样位，并对照独立的物理采样沿表，检查 8 位重建和 SCK 回到空闲。它检验课堂模型和表的一致性，没有模拟 setup/hold、抖动、亚稳态、线路、DMA 或控制器实现。

ASan/UBSan 编译运行命令：

```bash
gcc -std=c11 -O1 -g -Wall -Wextra -Wpedantic -Werror \
  -Wconversion -Wsign-conversion \
  -fsanitize=address,undefined -fno-sanitize-recover=all \
  -fno-omit-frame-pointer \
  examples/c/spidev_transaction.c tests/host_spidev_transaction_test.c \
  -o /tmp/embedded-spidev-test-sanitize
ASAN_OPTIONS=detect_leaks=0 /tmp/embedded-spidev-test-sanitize
```

本容器默认 LeakSanitizer 运行因 ptrace 限制报 fatal error；显式关闭泄漏检测后 ASan/UBSan 的上述有限用例通过，**不是泄漏检查通过**。本实现没有堆分配，但这不解除对整个程序、适配层与目标驱动的独立验证要求。

目标板验收清单保持未执行：固定实际芯片与内核/控制器驱动；用低速先核对四线和 CS 极性；捕获 CS/SCK/MOSI/MISO 并对照首尾边沿；检查命令到数据是否被切片、是否有允许范围外间隙、dummy 与应答偏移；测实际频率及 setup/hold；解释设备 ACK/status；再做拔除、超时、恢复和竞争拥有者测试。当前通过的是合成验证，不是硬件接通证明。

## 来源与适用范围

相关条款核对日期为 2026-10-11。固定 v6.6 是本页可复核软件基线，不表示“最新版”，也不保证所有目标 BSP 的行为一致。代码使用编译主机安装的 `/usr/include/linux/spi/spidev.h`，该文件 SHA-256 为 `c2ab0ce5101b50a2bbdf669b7b51c2c188d47257e163af78283562986c37a124`；主机没有可查询的 `linux-libc-dev` 包版本，不能为它指定 v6.6 来源。实际结构大小为 32 字节，字段是 tx_buf/rx_buf/len/speed_hz/delay_usecs/bits_per_word/cs_change/tx_nbits/rx_nbits/word_delay_usecs/pad，与本页所用 v6.6 接口字段一致。这次运行没有启动或构建 v6.6 内核。

- [Linux v6.6 include/uapi/linux/spi/spidev.h](https://raw.githubusercontent.com/torvalds/linux/v6.6/include/uapi/linux/spi/spidev.h)：实际核读 `spi_ioc_transfer` 字段、清零要求、指针/长度、MESSAGE(N) 和配置 ioctl 定义；不是自定义结构取代 UAPI。文件许可标识为 GPL-2.0+ WITH Linux-syscall-note；本页未复制上游实现或长注释。
- [Linux v6.6 SPI userspace API](https://www.kernel.org/doc/html/v6.6/spi/spidev.html)：核读 BASIC CHARACTER DEVICE API、FULL DUPLEX CHARACTER DEVICE API 与相关 NOTES；用于 mode、read/write 边界、dummy/full-duplex、同步及实际速度/低层 ACK 限制。
- [Linux v6.6 drivers/spi/spidev.c](https://github.com/torvalds/linux/blob/v6.6/drivers/spi/spidev.c)：核读 `spidev_message()` 的总长检查/同步/接收复制/成功计数，以及 `spidev_ioctl()` 有关配置与锁的分支。核查相关部分，不声称读完驱动、整个内核或所有错误路径。
- [Linux v6.6 driver-api SPI](https://www.kernel.org/doc/html/v6.6/driver-api/spi.html#c.spi_transfer)：核读 `spi_transfer` 的全双工/NULL buffer/CS 与 `spi_message` 的序列原子性说明；内核结构说明用于解释语义，不把内核 DMA 内存要求照搬成 userspace 指针合同。

正文、表格、代码和测试是本仓库原创教学补充。官方文件与文档的版权/许可归其权利人，引用公开入口不代表其作者背书；本页没有复制器件课程、图表或驱动实现。
