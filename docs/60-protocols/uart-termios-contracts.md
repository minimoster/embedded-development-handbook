# UART 电气帧格式与 Linux 串口合同

学习优先级：P2，可选补充。先读 [MC04 应用帧](../40-mcu/mc04.md)、[MC05 DMA 所有权](../40-mcu/mc05.md)、[PR03 RS485 与 RTU](pr03.md)、[LX02 部分 I/O](../70-linux-apps/lx02.md) 和 [LX12 子系统接口](../70-linux-apps/lx12.md)。本页只补电气/字符帧/流控及 Linux 配置边界，不重写消息 parser 或总线驱动。

目标是为一次串口事务列出完整、可检查的配置和退出合同，再说明每条验收证据的适用范围。正文、计算和课堂模型是原创；禁止抄录、拼接或冒名发布别人的课程、驱动代码和图表。公开芯片资料用于核对选定实例，不能从一个实例推导所有 UART 都如此。

## 直接回答

UART 规定异步字符发送/接收机制，RS232/RS485 是不同电气接口，应用协议还要另定消息边界。双方应一致约定 baud、数据位、校验、停止位和流控。Linux termios 负责终端属性；配置成功、write 返回、DMA 搬完及对端执行成功分别需要不同证据。

## 一 串并行与通信方向是两个维度

串行把同一方向的数据位按时间送过一条或少量 lane；并行在多条 lane 上同时送一组位。原创预算例：假设接口 A 有 8 条数据线，每次采样交付 8 位；接口 B 有 1 条数据线，每符号交付 1 位。若两者符号率相同且无其他开销，A 的原始位率是 B 的八倍；这个假设不证明实际 A 一定更快。B 可有更高符号率，双方也可能有不同编码和帧开销。

并行方案要付出数据引脚、布线和 lane 间偏斜预算；串行方案要付出串并转换和恢复/采样时序预算。距离、噪声容限、总吞吐和成本须落到具体电气、拓扑和时序，不能由“串行/并行”名称固定排名。TL16C550D 本身就有面向 CPU 的并行 D0–D7 接口，以及面向线路的串行 SIN/SOUT。[TI TL16C550D Rev E，功能框图与引脚说明](https://www.ti.com/lit/ds/symlink/tl16c550d.pdf)

单工只允许约定方向；半双工允许两方向但同一时刻只一方发送；全双工允许双方同时发送。串行 UART 的独立 TX/RX 可支持全双工，常见两线半双工 RS485 要轮流占用。并行/串行的 lane 数与单工/双工独立，先说明实际接线和收发器。[PR03](pr03.md)

## 二 逻辑 UART 不能直接接任意串口电压

MCU 引脚的 VDD、输入阈值、绝对最大值和是否容忍更高电压由芯片/引脚规定。UART 的逻辑高低不等于统一的“TTL 5 V”。RS232 有自己的电平与信号极性，经合适收发器才能连接逻辑侧；RS485 用差分电气且还涉及总线拓扑、方向和共模范围。[TI SLLA037A Rev A，TIA/EIA-232-F 电气说明](https://www.ti.com/lit/an/slla037a/slla037a.pdf)、[PR03](pr03.md)

原创连接检查表：记录逻辑侧芯片/引脚、收发器型号、电源、双方接口种类、TX/RX 对应关系、参考地或隔离边界，再核对规格。只有知道这些，才能讨论信号电平和接线；本页不提供可照抄到任意板子的电压值或接线图。

## 三 字符帧开销与校验能力

常见异步字符在空闲后用起始位同步，再送约定数量的数据位、可选校验和停止位。针对常见二进制 UART，一个符号是一位；payload 速率还要扣除字符开销。芯片支持的数据位/停止位/采样方式以型号文档为准。[TI TL16C550D Rev E，LCR 与收发时序](https://www.ti.com/lit/ds/symlink/tl16c550d.pdf)

原创计算：8N1 的每个字符有 1 起始+8 数据+1 停止，共 10 bit。115200 baud、连续无间隙时的理论 payload 上限是 11520 byte/s，字符约 86.81 μs；8E1 增加校验位，成为 11 bit，理论约 10472.73 byte/s。这里没算帧头/CRC、间隙、流控暂停、驱动队列和应用调度，所以不能拿这个上限当实测吞吐或业务 deadline。

偶/奇校验只检查相应位集合的奇偶性：奇数位翻转可被检测，偶数位翻转可能漏检；它不定位或修正坏位，更不确认完整业务消息。这个结论是 parity 运算的性质，不是芯片自动提供端到端可靠性。接收器的 framing、parity、overrun 状态仍要分开计数。

双方 nominal baud 相同也要核对时钟误差、分频取整、温漂和接收采样策略。原创简化模型：若接收位周期与发送位周期相差 2%，从起始同步到 9.5 个发送位周期附近会累计约 0.19 个位周期偏移；这仅示意误差累计，不是“所有 UART 允许 2%”或实际容差计算。目标验收要用型号采样规则、起始沿误差和完整时钟预算。

STM32F4 HAL v1.8.3 的 word length 与 parity 组合有特定含义：开启 parity 时，所选 word length 中的最高位被校验位占用。因此“8 位 word length 加 parity”不能无条件解释成 8 位 payload 加校验。[ST UART 驱动 v1.8.3，帧格式说明](https://raw.githubusercontent.com/STMicroelectronics/stm32f4xx-hal-driver/v1.8.3/Src/stm32f4xx_hal_uart.c)

## 四 硬件和软件流控都要有暂停上界

RTS/CTS 让接收侧按容量控制发送侧继续送字符；线要正确连接，控制器和驱动要支持，极性和生效边界查选定器件。TL16C550D 的 auto-CTS 影响后续字符，并非把已开始的字符瞬间截断。流控暂停还需保留在途字符容量。[TI TL16C550D Rev E，Autoflow Control](https://www.ti.com/lit/ds/symlink/tl16c550d.pdf)

XON/XOFF 用带内字符表达暂停/继续，会占用或解释相应字节；任意二进制协议不能在未约定转义/保留规则时误开它。Linux 驱动与 TTY 层共同实现流控，应用仍要规定队列满、长时间暂停和取消行为。[Linux 6.15 Serial API 的 stop_tx/send_xchar](https://docs.kernel.org/6.15/driver-api/serial/driver.html)

原创例：最大待发 1024 字节，远端暂停后新遥测只保留最新采样；控制事务不悄悄丢弃，队满则拒绝。超过整体期限转到“发送进度/执行结果不确定”的失败路径，再按协议恢复。流控解决容量协调，不能替代帧校验、认证或业务回执。

## 五 Linux 配置要表达完整预期

一个串口拥有者负责配置、读写和恢复，避免另一个进程在事务中途修改共享设备状态。先确认目标设备与权限，打开时明确 O_NOCTTY、CLOEXEC 与非阻塞策略；成功取得 fd 后保存 tcgetattr 原值。不要清零一个 termios 就默认所有扩展状态正确，更不能只设置 baud 和 CS8。[GNU libc 2.41 Terminal Modes](https://github.com/bminor/glibc/blob/glibc-2.41/manual/terminal.texi)

下面是课堂二进制 8N1、无流控、外部 deadline 模式的显式检查合同，不是任意真实设备的推荐配置：

| 属性 | 明确的预期 |
|---|---|
| 输入 | IGNBRK/BRKINT/PARMRK/INPCK/IGNPAR/ISTRIP/INLCR/IGNCR/ICRNL/IXON/IXOFF/IXANY 全部关闭；保留 8 位、不转换 CR/NL、不启用带内流控或错误标记 |
| 输出 | OPOST 关闭，不做输出字符加工 |
| 本地 | ICANON、ECHO 系列、ISIG、IEXTEN 关闭 |
| 控制 | 先清 CSIZE 再选 CS8；PARENB/PARODD/CSTOPB/CRTSCTS 关闭，CREAD/CLOCAL 开启、HUPCL 关闭；若有 CMSPAR 也清除 |
| 速度 | 用 cfsetispeed/cfsetospeed 设置双方认可的 B 常量，检查返回；B0 有挂断语义，不是任意低速占位值 |
| 等待 | VMIN=0、VTIME=0；非阻塞 read/write 由外部单调 deadline 和就绪等待管理 |

原始模式快捷函数不是完整设备合同：还要确认速度、流控、载波、挂断、VMIN/VTIME 和错误策略。字段含义见 [固定快照 termios 模式标志](https://github.com/mkerrisk/man-pages/blob/ae6b221882ce71ba82fcdbe02419a225111502f0/man3/termios.3)，控制/错误标志与驱动的关系见 [Linux 6.15 set_termios](https://docs.kernel.org/6.15/driver-api/serial/driver.html)。这是无 modem 课堂端点的明确选择：忽略载波，关闭时不请求 HUPCL 挂断；break 在此模式下可成为 NUL，8N1 不启用 parity 检查。需要错误标记、modem 或另一协议时重新定完整合同，不能沿用这张表。

配置调用后再次 tcgetattr，比较要求实际生效的字段；不支持的设置应报错/拒绝，不宣称硬件已采用。TCSANOW 立即变更，TCSADRAIN 等输出，TCSAFLUSH 还丢弃未读输入；不要为了“初始化干净”无条件丢弃可能已到达的应答。[GNU libc 2.41 Mode Functions](https://github.com/bminor/glibc/blob/glibc-2.41/manual/terminal.texi)

原创生命周期：打开→保存原值→申请配置→读回核对→运行→停止提交→处理未完成事务→尝试恢复原值→记录恢复错误→关闭。配置失败也走恢复/关闭路径，不能只照顾正常退出。SIGKILL、拔除设备等情况下恢复可能无法完成；下一拥有者仍须显式配置。

## 六 read 返回零与等待方式要分开解释

非规范模式的四种 VMIN/VTIME 组合分别是立即读取、至少等 MIN、从调用开始的定时等首字节、首字节之后的字符间计时。VTIME 单位是 0.1 秒；最后一种不能限制无限等待首字节。本页 Linux v6.15 N_TTY 路径先检查等待期限及非阻塞状态，空读可返回零或 EAGAIN；不要跨平台套用其顺序，也不能把串口零返回直接当 TCP EOF。[Linux v6.15 N_TTY 的 n_tty_read/n_tty_wait_for_input](https://github.com/torvalds/linux/blob/v6.15/drivers/tty/n_tty.c)

原创外部期限模式避免把 VTIME 当总事务时钟：每次循环重算 deadline 剩余时间；read 正值交给独立 parser，零/EAGAIN 返回等待并按设备状态处理，EINTR 保持剩余预算，其他错误停止。write 正值推进 offset，短写后只发剩余部分；零进度也不得忙循环。端口消失、HUP/ERR、取消和超时有独立状态，不自动当“没有新字符”。完整 parser 沿用 [MC04](../40-mcu/mc04.md)，不凭一次 read 宣告消息完成。

tcdrain 的 API 合同是等待输出传输完成，但它是可能阻塞的等待；受流控暂停时尤其不能无界调用在必须按期退出的路径上。驱动与 USB 串口转换器怎样定义并报告线路完成还需目标验证；它不证明远端已处理命令。[tcdrain(3)](https://man7.org/linux/man-pages/man3/tcdrain.3.html)

## 七 拿具体设备区分三种发送完成

CPU 提交字节、DMA 读取完 buffer、UART 最后字符离开移位寄存器是三个不同事件。STM32F4 HAL v1.8.3 正常 DMA 发送完成路径撤销 DMA 请求并启用 UART TC 中断，随后才走 UART 完成处理；循环 DMA 走另一条回调路径。不能只听到 callback 名称就释放 RS485 方向。寄存器名称、缓冲回收点和方向时序必须回到所选器件/驱动。[ST v1.8.3 UART_DMATransmitCplt 与 UART_EndTransmit_IT](https://raw.githubusercontent.com/STMicroelectronics/stm32f4xx-hal-driver/v1.8.3/Src/stm32f4xx_hal_uart.c)

Linux 6.15 serial uart_ops.tx_empty 的合同涉及 FIFO 与 shifter；不支持查询的驱动还可按该接口约定返回空状态。因此应用不能把一条抽象状态位当所有 USB/UART 线路的示波器证据。[Linux 6.15 tx_empty](https://docs.kernel.org/6.15/driver-api/serial/driver.html)

16550 类控制器的 RBR/THR、LCR、LSR 与分频寄存器只是具体寄存器模型：RBR 接收、THR 提交发送、LCR 格式、LSR 状态；部分地址还按访问方向/配置位复用。不能把这些名称或位值搬到 STM32。[TI TL16C550D Rev E，寄存器说明](https://www.ti.com/lit/ds/symlink/tl16c550d.pdf)

## 练习与验收

1. 计算 8N1 与 8E1 的字符时长，再给业务帧预算加上协议开销及间隙；只把结果称为计算上限。
2. 纯主机模型验证奇数/偶数位错误、队列上限、部分写进度和“DMA 完成但 line 未完成”状态。
3. 自建 PTY 保存/配置/读回/恢复属性，把含 NUL、CR/LF、0x11、0x13、0xFF 的数据从 master 送往 slave，确认没有被本实验的终端处理改变；没有真实端口也可练生命周期。
4. 目标硬件另测 baud 误差、校验/overrun、CTS 暂停与恢复、尾停止位及方向释放。任何缺少目标设备的项目都保持未验证。

PTY 没有 UART 位时钟、实际电压、物理校验错误、RTS/CTS 接线或 DMA。它验证的是当前主机终端属性/字节流和恢复路径，不是串口硬件或电气安全认证。本页不涵盖所有 UART 控制器、串口协议及线缆条件。

## 来源与适用范围

- TI TL16C550D：SLLS597E，Rev E，2008-12；只作为明确型号的异步帧、流控与寄存器实例，不推荐照抄其板级配置。
- TI SLLA037A：Rev A，TIA/EIA-232-F 接口说明；不是 MCU 逻辑引脚规格。
- GNU libc 2.41 manual/terminal.texi、man-pages 快照 `ae6b221882ce71ba82fcdbe02419a225111502f0` 与 Linux v6.15 N_TTY/serial 文档固定软件版本；tcdrain 链接的 man-pages 版记为 6.19。具体部署还要核对目标 libc、内核与驱动能力。
- ST STM32F4 HAL driver v1.8.3：明确区分正常/循环 DMA 路径。型号、HAL 与 DMA 配置不同，行为应重新核对。

本页没有复制官方代码、规范算法或线路图；公开资料的作者与机构不为本页原创课堂合同背书。
