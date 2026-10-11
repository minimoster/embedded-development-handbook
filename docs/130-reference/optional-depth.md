# 可选深化讲义 · 在已有主线之后补接口合同

这16篇原创选读补充已经有基础的学习者，按项目需要进入。它们单独计数，不新增核心题号：120核心主题、官方Hot100的100题和1道额外算法题、7张故障场景、7篇FreeRTOS源码导读仍使用各自清单。学习优先级是依赖建议，不是招聘概率。

## 按主方向选读

- [FreeRTOS 任务诊断与运行时集成](../50-rtos/task-diagnostics.md) · P1 · 先修 RT08 / RT10
- [可选扩展  从算法输入到目标设备结果的部署验证](../110-projects/edge-inference-validation.md) · P2 · 先修 PJ05
- [可选扩展  GUI 存储网络与媒体的集成边界](../70-linux-apps/integration-stacks.md) · P2 · 先修 LX17
- [MCU 时钟层级与 SysTick 时间基准](../40-mcu/clock-systick-contracts.md) · P2 · 先修 MC02 / AR04 / RT02
- [可选诊断补充 · 用崩溃快照建立可复核的证据链](../100-debugging/linux-postmortem-evidence.md) · P2 · 先修 DB02 / LX08
- [可选合同补充 · 队列操作、调试名称与 queue sets](../50-rtos/queue-operation-contracts.md) · P2 · 先修 RT05
- [socket 生命周期与 WebSocket 消息合同](../70-linux-apps/socket-websocket-contracts.md) · P2 · 先修 PR05 / PR08 / LX03 / LX18
- [UART 电气帧格式与 Linux 串口合同](../60-protocols/uart-termios-contracts.md) · P2 · 先修 MC04 / MC05 / PR03 / LX12
- [RTOS 内存策略：从空闲块选择到释放合同](../50-rtos/memory-policy-contracts.md) · P2 · 先修 C07 / RT08 / RT10

- [I²C 时序预算、上拉区间与有界总线恢复](../60-protocols/i2c-timing-recovery.md) · P2 · 先修 PR01 / LX12
- [SPI 四模式逐边沿与 spidev 命令数据事务](../60-protocols/spi-edges-spidev.md) · P2 · 先修 PR02 / LX12
- [TCP Reno 与 RTO · 从累计 ACK 手推一次有限恢复](../60-protocols/tcp-reno-rto.md) · P2 · 先修 PR06 / PR07 / PR08 / PR12

- [发布完成之后，谁还能碰这块内存？C++17 双向所有权交接实验](../10-c-cpp/publication-ownership-contracts.md) · P1 · 先修 C04 / C08 / C09 / RT09
- [NVIC 优先级编码与 FreeRTOS ISR 边界审计](../50-rtos/nvic-priority-audit.md) · P1 · 先修 AR04 / RT06 / RT07 / MC05
- [可选深化 · Linux DMA 的所有权、取消与 remove 生命周期合约](../80-linux-drivers/dma-lifecycle-contracts.md) · P1 · 先修 DV05 / DV09 / MC06
- [Cortex-M4 HardFault 可信帧与离线解码](../100-debugging/cortex-m4-fault-frame-contracts.md) · P1 · 先修 DB01 / AR07 / RT08 / MC06

任务诊断需与[事件组追踪编号](../50-rtos/kernel-event-groups.md)一起理解对象寿命；[队列操作补充](../50-rtos/queue-operation-contracts.md)接在[队列等待主线](../50-rtos/kernel-queue-waits.md)之后。[timer daemon](../50-rtos/kernel-timer-daemon.md)增加恢复调度、启动钩子与基础设施创建失败的窄补充，不改变原有阅读路线。

## 验证与适用边界

讲义区分源码核读、手推、有限主机模型与真实目标执行。新增协议三章限定为I²C时序/上拉与有界恢复、SPI逐边沿/单消息事务、经典Reno/RTO有限恢复；规范和合成主机验证不代替板级电气、Linux内核或真实网络实验。时钟和内存的整数模型、合成ELF的证据策略、Linux本地socket/PTY练习及小型应用示例只证明各自执行路径；队列和timer补充只有源码/文字核对。没有运行FreeRTOS内核、选定MCU、电气接口、真实core/GDB、推理SDK、GUI/媒体框架或生产网络集成。每章保留具体版本、配置、测试和未执行项目。

这些补充没有完成所有专门方向。七层职责与封装推演、完整TCP实现、多缺口/SACK恢复及TLS详细状态、网络配置与诊断、安全API及完整协议工程、GD32/STM32启动文件版本差异、Linux命令和传统daemon、内核调度/内存管理与完整驱动、USB/PHY移植及长期板级验证仍需继续展开。GUI/媒体和模型部署讲义介绍接口与验证合同，不能代替实际库与目标设备结果。

## 本轮四个深度实验

[公开问法与缺口审计](interview-depth-refresh.md)将平台自述、题单、推广/试读和未取得正文分别记录。四章补C++双向发布/复用、NVIC优先级编码/ISR门槛、Linux DMA取消/移除及M4可信fault帧；主机整数、显式事件和合成副本与目标执行分别计证据。没有完成多端并发、完整驱动或板级故障采集。

## 回到主线

- [学习路线](../00-guide/learning-path.md)
- [FreeRTOS源码导读](../50-rtos/source-reading.md)
- [完整题目索引](../../QUESTION_INDEX.md)
- [覆盖边界](coverage-boundaries.md)与[实际验证说明](../../coverage/README.md)

正文和模型是原创教学内容。公开官方资料只作为核对入口；其版权、商标和许可归对应权利人，现代与历史FreeRTOS许可边界保持独立。
