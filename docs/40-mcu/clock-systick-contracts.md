# MCU 时钟层级与 SysTick 时间基准

学习优先级：P2，可选补充阅读，不新增核心题编号。先读 [MC02 · 定时器频率](mc02.md) 和 [AR04 · 中断优先级](../30-architecture/ar04.md)。本章补齐“频率从哪里来、一次 tick 代表什么”的基础合同；PWM、输入捕获、DMA 和任务列表机制沿用现有章节。

## 直接回答

计算之前先写出目标芯片、时钟源、分频路径和实际计数器输入。SYSCLK 是所选系统时钟；HCLK、PCLK1、PCLK2 是后续派生时钟。在这里选读的 STM32F4 系统文件中，`SystemCoreClock` 表示内核时钟 HCLK，经过 AHB 分频后才得到它，不能始终令 CPU 频率等于 SYSCLK。[固定系统文件](https://github.com/STMicroelectronics/cmsis-device-f4/blob/9192c7b9df75a142f2027ab266601fe061fc00b3/Source/Templates/system_stm32f4xx.c#L187-L267)

SysTick 是 Cortex-M4 的 24 位可重装向下计数器。正常连续计数时，若输入频率为 f、`LOAD=N−1`，周期为 N/f。计数到零、中断处理、请求切换和任务实际运行是不同时间点；一个“1 ms tick”不能证明任务每隔 1 ms 精确开始工作。[Arm 硬件合同](https://documentation-service.arm.com/static/5f2ac4ab60a93e65927bbdbf#page=249)

## 原理拆解

### 1. 时钟源 总线时钟 外设工作时钟

用三个问题逐层检查：

1. 源来自哪里？内部振荡器、外部时钟或 PLL 输出都可能参与选择；PLL 配置还受该芯片的输入和输出约束。
2. 到这里经过什么分频？在本章的 STM32F407 例子中，HCLK=SYSCLK/AHB 分频，PCLK1=HCLK/APB1 分频，PCLK2=HCLK/APB2 分频。
3. 该外设真正用哪个时钟工作？寄存器接口所在总线，不一定就是计数、采样或协议工作逻辑的时钟来源。须查具体外设的工作时钟，常称 kernel clock，以及后续分频、门控和选源。

对 STM32F405/407/415/417 的 RM0090 路径，APB 分频为 1 时，内部定时器时钟等于相应 PCLK；分频大于 1 时为该 PCLK 的两倍。这个倍率只用于相应定时器路径，不能乘给所有 APB 外设。其他型号可能有 TIMPRE 或独立选源，必须重查。SysTick 在本选定目标上可选 HCLK 或 HCLK/8；架构所说的外部参考输入，在这里并不等于另接一颗晶振。[RM0090 Rev 21，第 7.2 节，第 219 页](https://www.st.com/resource/en/reference_manual/dm00031020-stm32f405-407-415-417-437-455-469-application-note-stmicroelectronics.pdf#page=219)

`SystemCoreClockUpdate()` 根据 RCC 状态、分频和项目中的源频率常量更新软件变量，没有测量晶振。在本 HAL 快照中，`HAL_RCC_GetHCLKFreq()` 只是返回 `SystemCoreClock`；单独调用这个 getter 不会刷新过时变量。PCLK getter 又依赖它，因此一个错误缓存可传到多个后续计算。[系统变量的计算前提](https://github.com/STMicroelectronics/cmsis-device-f4/blob/9192c7b9df75a142f2027ab266601fe061fc00b3/Source/Templates/system_stm32f4xx.c#L187-L267)、[HCLK 与 PCLK getter](https://github.com/STMicroelectronics/stm32f4xx-hal-driver/blob/1f6451c3e07728b4c830744de380e56bf5bc0026/Src/stm32f4xx_hal_rcc.c#L934-L970)

### 2. SysTick 的寄存器分别负责什么

以下是 Cortex-M4 的语义摘要，不是给运行中固件重配 tick 的代码：

- `LOAD` 的有效值域为 24 位，保存重装值
- `VAL` 是当前向下计数值；写任意值会清零当前值和 COUNTFLAG，不是写入指定起点
- `CTRL.ENABLE` 控制计数；`CTRL.TICKINT` 控制计数到零时的异常请求；`CTRL.CLKSOURCE` 选择输入路径
- COUNTFLAG 是读后清除的到零标记，不能作为完整周期累计器；读 `CTRL` 有副作用，观察工具也要谨慎

正常重复中断要求 `1≤LOAD≤0xFFFFFF`，即 `2≤N≤2^24`。`LOAD=0` 没有产生事件所需的 1→0 转换。CMSIS v6.0.0 的默认 `SysTick_Config(1)` 会通过寄存器范围检查并返回成功，却不能据此认定硬件会每个输入周期中断。该函数还会选择内核时钟、启用中断并设置 SysTick 优先级，不能把它当作仅修改频率的工具。[Arm SysTick 寄存器与零值边界](https://documentation-service.arm.com/static/5f2ac4ab60a93e65927bbdbf#page=249)、[固定 CMSIS 定义与配置函数](https://github.com/ARM-software/CMSIS_6/blob/1da2c5d27af1f93b2a047ad3db5e563dd83f31ec/CMSIS/Core/Include/core_cm4.h#L762-L811)、[SysTick_Config](https://github.com/ARM-software/CMSIS_6/blob/1da2c5d27af1f93b2a047ad3db5e563dd83f31ec/CMSIS/Core/Include/core_cm4.h#L2017-L2043)

### 3. 期望 tick 与实际周期之间的取整

设期望频率为 r Hz，简单整数实现采用 `N=f/r` 的向下取整，再写 `LOAD=N−1`。应先检查 f、r 为正，N 满足上述范围，再相减，避免零除、无符号下溢和超出 24 位。实际频率为 f/N，实际周期为 N/f。若不能整除，向下取整会缩短周期；换用其他舍入策略也必须重做范围和误差检查。[本选读 FreeRTOS port 的整数配置路径](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/portable/GCC/ARM_CM4F/port.c#L810-L832)

原创算例：假设输入为 1 MHz，希望得到 1024 Hz。N=976，LOAD=975；实际周期 976 μs，期望周期 976.5625 μs，每周期短 0.5625 μs，实际频率约 1024.590164 Hz。这个误差来自整数计数，时钟源本身的误差还需另外计入。由 f 和 N 推导的数值，不是板测结果。

输入时钟分辨率 1/f、软件 tick 粒度 N/f、中断延迟和任务调度延迟要分别记录。提高 r 会增加需要处理的 tick 事件数量，也可能使整数误差占比变大；不能仅凭 tick 更密就声称实时响应更好。任务延时取整与周期释放继续读 [RT02](../50-rtos/rt02.md)，外部边沿时间戳继续读 [MC03](mc03.md)。

### 4. 到零 中断处理 请求切换

在现有源码章节限定的单核 FreeRTOS V11.1.0 GCC ARM_CM4F 主线上，SysTick handler 调用 `xTaskIncrementTick()`；当它要求重新调度时，handler 将 PendSV 置为 pending。现场切换由 PendSV handler 完成。在该 port 的启动配置中，SysTick 与 PendSV 都设为最低中断紧迫性；这个配置不适用于任意 HAL 工程或任意 RTOS 移植层。[启动优先级](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/portable/GCC/ARM_CM4F/port.c#L430-L440)、[tick 与 PendSV 请求](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/portable/GCC/ARM_CM4F/port.c#L562-L586)

如果 SysTick 尚不能被服务，pending 状态不保存每个到零事件的数量。原创受限模型：计数器周期为 4 个输入时钟，服务始终被阻止；经过 16 个时钟发生 4 次到零，COUNTFLAG 仍只表示发生过，pending 仍只表示存在请求。解除阻止后的一次 handler 调用，不能从这两个标记恢复“4”。读掉 COUNTFLAG 也不等于清掉异常 pending；它们是不同状态。[异常 pending 位](https://documentation-service.arm.com/static/5f2ac4ab60a93e65927bbdbf#page=230)

因此，长屏蔽或长期被更紧迫 ISR 占用可能丢失可区分的 tick 事件，不能保证退出后逐个补齐。调度器挂起时累计已服务 tick 的软件机制，与尚未进入 handler 的硬件事件合并是两件事。tickless 有专门的计时和补偿路径，本章不验证它。任务状态迁移、BASEPRI 合同和现场保存分别复习 [任务生命周期](../50-rtos/kernel-task-lifecycle.md)、[RT06](../50-rtos/rt06.md)、[AR07](../30-architecture/ar07.md)。

### 5. 先确定时间基准由谁管理

应用必须确认：哪个组件配置时钟和重装值，谁安装 handler，谁推进 HAL 时间和 RTOS tick，以及每个计数的单位。在本 HAL 快照中，默认弱实现使用 SysTick；`HAL_Init()` 会初始化它，`HAL_RCC_ClockConfig()` 更新时钟后也会调用 `HAL_InitTick()`。项目可以覆盖这些实现。若 RTOS 已管理该计数器，额外调用默认 HAL 路径或自行写寄存器可能改变频率、优先级和相位，不能照抄独立裸机延时初始化。[HAL 时间基准入口](https://github.com/STMicroelectronics/stm32f4xx-hal-driver/blob/1f6451c3e07728b4c830744de380e56bf5bc0026/Src/stm32f4xx_hal.c#L237-L274)、[时钟重配后的调用](https://github.com/STMicroelectronics/stm32f4xx-hal-driver/blob/1f6451c3e07728b4c830744de380e56bf5bc0026/Src/stm32f4xx_hal_rcc.c#L718-L724)

不要假定 HAL 的毫秒计数和 RTOS tick 可以无条件共用一次回调。共用或分开都要由集成层给出明确频率、单位和调用合同。在 ISR 中等待一个不能抢占自己的时间基准，可能一直等不到时间推进；优先使用事件转交，不把阻塞延时当作修复办法。[HAL 的等待实现与注意事项](https://github.com/STMicroelectronics/stm32f4xx-hal-driver/blob/1f6451c3e07728b4c830744de380e56bf5bc0026/Src/stm32f4xx_hal.c#L237-L249)、[HAL_Delay](https://github.com/STMicroelectronics/stm32f4xx-hal-driver/blob/1f6451c3e07728b4c830744de380e56bf5bc0026/Src/stm32f4xx_hal.c#L379-L404)

动态改变输入频率但保留 LOAD，会改变秒单位的周期；只更新 `SystemCoreClock` 不会自动改变硬件重装值。即使重新计算 LOAD，也要处理跨切换窗口的相位、既有超时和其他外设时序。重配必须走当前时间基准管理者认可的路径。输入时钟停止时不能继续按原周期推断时间；本章不对任何低功耗模式、唤醒或功耗作实测结论。

## 例子与推演

选择 STM32F407 的上述路径，**假设已经获得 SYSCLK=96 MHz**，AHB 分频 2，APB1 分频 4，APB2 分频 2。这是一份原创算术预算，没有验证 PLL、电源、Flash 等条件，也不是可直接烧录的时钟配置。

| 位置 | 推导 | 预算结果 |
| --- | --- | --- |
| HCLK 与内核 | 96 MHz / 2 | 48 MHz |
| PCLK1 | 48 MHz / 4 | 12 MHz |
| PCLK2 | 48 MHz / 2 | 24 MHz |
| APB1 内部定时器路径 | 2 × 12 MHz | 24 MHz |
| APB2 内部定时器路径 | 2 × 24 MHz | 48 MHz |
| SysTick 内核输入 | HCLK | 48 MHz |
| SysTick 参考输入 | HCLK / 8 | 6 MHz |

要得到 1 kHz，内核输入需要 N=48000、LOAD=47999；参考输入需要 N=6000、LOAD=5999。误把 SYSCLK 的 96 MHz 代入内核输入会写 LOAD=95999，实际得到 2 ms 周期。

24 位重装范围给出的最长连续周期是 `2^24/f`。在这份预算中，48 MHz 输入约为 0.349525333 s，6 MHz 输入约为 2.796202667 s。因此 1 Hz 的单个周期无法用 48 MHz 输入容纳，而 6 MHz 输入的 N=6000000 能落在范围内。这个对比只说明算术可表示性，不授权改变现有工程的时钟选源。

再假设输入由 48 MHz 降到 24 MHz，而 LOAD 仍为 47999：稳态周期从 1 ms 变成 2 ms。软件变量即使已更新为 24 MHz，计数器也不会因此自动恢复 1 ms。

## 常见误区与失败边界

- SYSCLK、HCLK、PCLK 和外设工作时钟混填；频率来自不同节点，答案可能恰好差两倍
- 函数返回成功就认为时间正确；寄存器值域、实际时钟、优先级、handler 路由和管理者仍需检查
- COUNTFLAG 为 1 就把它解释为恰好一个周期；它无法区分一个或多个未观察周期
- 每次 tick 都立即切换任务；本选定 port 只有需要重新调度时才请求 PendSV，随后还要等待可服务时机
- 用“CPU 更快”推断 DMA 更快或波特率不变；数据通路与外设时钟不同，分别回到 [MC05 · DMA](mc05.md) 和 [MC04 · UART 帧](mc04.md) 检查合同

## 面试追问

“SYSCLK 没变，CPU 和 tick 为什么变慢？”答：先查 AHB 分频及 SysTick 实际输入，别只看系统时钟选源。

“关中断后恢复，RTOS 会自动补齐这段时间吗？”答：先区分 handler 已服务但调度被推迟，还是 handler 一直未被服务；硬件标记不是丢失周期计数器。需要可靠长时计时，应选定能覆盖该窗口的时间源和集成策略，再验证。

## 动手练习与验收

本章已执行原创主机算术与受限状态模型测试：时钟分层、APB 定时器两种分频分支、精确和非整除 tick、零和一周期拒绝、24 位最大值及越界、输入变化、计数关闭、无异常请求时继续计数、COUNTFLAG 读清、VAL 写清，以及多次到零合并为一个未服务请求。模型使用软件状态表达合同，没有运行 CMSIS 函数、ARM 指令、FreeRTOS 内核或硬件外设。

1. 更换 AHB 与 APB 分频，重新列预算表；注明每个节点与倍率适用的具体目标。
2. 对期望频率先给出 N、LOAD、实际频率和误差，再判断能否表示；不要先写寄存器再发现下溢。
3. 在模型中暂缓服务多个周期，说明哪些历史仍可观察、哪些已无法恢复。
4. 若后续安排目标验证，先核对时间基准管理者和源码版本，再设计独立的周期、ISR 延迟和任务响应测量；未经这些步骤不能报告板级时序通过。

验收：每个频率有路径，每个时间结论有单位，每次切换有触发条件，未执行项目明确保留。主机模型通过只支持它自己的断言，不构成 MCU 仿真、实际 IRQ、时钟精度或功耗验证。

## 来源与适用范围

- Arm Cortex-M4 Generic User Guide DUI 0553A：第 4.4 节及第 4.3.2 节；核对 SysTick 和异常 pending 的硬件语义
- CMSIS v6.0.0：本章链接固定到提交 `1da2c5d27af1f93b2a047ad3db5e563dd83f31ec`，默认函数可被厂商实现替换
- ST device 与 HAL 链接分别固定到 `9192c7b9df75a142f2027ab266601fe061fc00b3` 和 `1f6451c3e07728b4c830744de380e56bf5bc0026`；这些是独立阅读快照，未证明是兼容发行组合或项目实际 BSP
- RM0090 Rev 21：仅核对上述相关时钟段落，用于 STM32F407 路径；未完整阅读手册，不给出全系列频率上限
- FreeRTOS V11.1.0 的固定 ARM_CM4F port 与现有 [源码导读](../50-rtos/source-reading.md) 保持一致；其他 port、SMP、厂商改动与 tickless 需另查

正文、算例和模型为原创教学内容，未复制所引资料的源码或图表。来源用于核对合同，不表示维护者为本讲义背书。
