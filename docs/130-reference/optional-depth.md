# 可选深化讲义 · 在已有主线之后补接口合同

这9篇原创选读补充已经有基础的学习者，按项目需要进入。它们单独计数，不新增核心题号：120核心主题、官方Hot100的100题和1道额外算法题、7张故障场景、7篇FreeRTOS源码导读仍使用各自清单。学习优先级是依赖建议，不是招聘概率。

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

任务诊断需与[事件组追踪编号](../50-rtos/kernel-event-groups.md)一起理解对象寿命；[队列操作补充](../50-rtos/queue-operation-contracts.md)接在[队列等待主线](../50-rtos/kernel-queue-waits.md)之后。[timer daemon](../50-rtos/kernel-timer-daemon.md)增加恢复调度、启动钩子与基础设施创建失败的窄补充，不改变原有阅读路线。

## 验证与适用边界

讲义区分源码核读、手推、有限主机模型与真实目标执行。时钟和内存的整数模型、合成ELF的证据策略、Linux本地socket/PTY练习及小型应用示例只证明各自执行路径；队列和timer补充只有源码/文字核对。没有运行FreeRTOS内核、选定MCU、电气接口、真实core/GDB、推理SDK、GUI/媒体框架或生产网络集成。每章保留具体版本、配置、测试和未执行项目。

这些补充没有完成所有专门方向。七层职责与封装推演、TCP重传/TLS详细状态、网络配置与诊断、安全API及完整协议工程、GD32/STM32启动文件版本差异、Linux命令和传统daemon、内核调度/内存管理与完整驱动、USB/PHY移植及长期板级验证仍需继续展开。GUI/媒体和模型部署讲义介绍接口与验证合同，不能代替实际库与目标设备结果。

## 回到主线

- [学习路线](../00-guide/learning-path.md)
- [FreeRTOS源码导读](../50-rtos/source-reading.md)
- [完整题目索引](../../QUESTION_INDEX.md)
- [覆盖边界](coverage-boundaries.md)与[实际验证说明](../../coverage/README.md)

正文和模型是原创教学内容。公开官方资料只作为核对入口；其版权、商标和许可归对应权利人，现代与历史FreeRTOS许可边界保持独立。
