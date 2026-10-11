# Cortex-M4 HardFault 可信帧与离线解码

学习优先级：P1。本文是可选深度实验，不新增核心题号。先读 [DB01](db01.md)、[AR07](../30-architecture/ar07.md)、[RT08](../50-rtos/rt08.md) 与 [MC06](../40-mcu/mc06.md)。本章解决“已经拿到 PC、LR、CFSR，但这些数字到底能说明什么”，不提供可以不经板级审查就复制到产品的异常处理器。

## 直接回答

### 30 秒回答

HardFault 是异常入口，未必是最初的错误类型。先保存原始状态，再根据异常入口的 EXC_RETURN 确认原上下文使用 MSP 还是 PSP，以及基本帧还是浮点扩展帧；检查被复制的栈范围和 fault 有效位后，才解释堆栈 PC 与地址。FORCED 表示可配置 fault 升级，需要继续读 CFSR。BFAR/MMFAR 只有对应 valid 位有效时才是地址线索。IMPRECISERR 的堆栈 PC 不能直接当成出错指令。

### 2 分钟回答

我会先保护证据：固件构建身份、异常入口 LR、MSP/PSP、CFSR/HFSR、有效地址寄存器、限定长度的栈副本和运行前的最近事件。异常处理器的当前 PC 通常指向处理器自身，不等于故障上下文 PC；处理器普通 C 代码里的当前 LR 也可能已被函数调用改写。

帧解析必须知道版本。在本文 Cortex-M4 布局中，核心的八个字仍从入口帧 SP 开始，PC 在 `SP+0x18`；浮点扩展增加帧总大小，不是把核心 PC 一律向后跳 18 个字。xPSR[9] 指示额外的对齐字，恢复前 SP 要把它算进去。浮点 lazy 保存还意味着预留位置不一定已有有效的浮点值。

若存在堆栈、退栈或 lazy 浮点保存 fault，我不会自动把副本认证为完整可信帧。之后用准确的 ELF/构建身份解释候选 PC，核对反汇编、寄存器和地址，再设计能证伪假设的最小复现与故障注入。离线解码成功不代表根因已经找到，更不代表目标异常采集代码已经安全。

## 固定资料与实验范围

- Cortex-M4/Armv7E-M；ST PM0214 Rev10（March2020）的异常、CCR、CFSR/HFSR 与有效地址位部分
- Arm 应用说明 DAI0298A（16March2012）的 Cortex-M4F lazy 浮点保存说明
- CMSIS-Core6.1.0 `core_cm4.h` 的六种 Cortex-M4 EXC_RETURN 常量、fault 位和 FPCCR 定义
- Arm CMSIS-RTX5.9.0 的 ARMv7-M SVC 入口，作为原始栈选择和核心 PC 固定偏移的源代码交叉核对
- 原创 Python3.10+ 标准库模型：只解释调用者提供的整数和最多64字的不可变合成副本；不读取任何目标内存

没有运行目标板、模拟器、FreeRTOS/RTX 或异常注入。本章没有编译板级 naked 汇编、验证断电保留区、执行符号工具或自动恢复。Cortex-M0/M0+、M7、Armv8-M安全扩展和 Cortex-A/R 不纳入该解码合同，应换读实际架构与芯片资料。[ST PM0214](https://www.st.com/resource/en/programming_manual/pm0214-stm32-cortexm4-mcus-and-mpus-programming-manual-stmicroelectronics.pdf)、[Arm DAI0298A](https://documentation-service.arm.com/static/5f0352c0cafe527e86f5bc66)、[CMSIS 固定版本](https://github.com/ARM-software/CMSIS_6/blob/v6.1.0/CMSIS/Core/Include/core_cm4.h)、[RTX 固定入口](https://github.com/ARM-software/CMSIS-RTX/blob/v5.9.0/Source/GCC/irq_armv7m.S)

## 先分开四个 PC/LR 概念

| 数字 | 实际意义 | 容易误判的地方 |
| --- | --- | --- |
| 调试器停在的当前 PC | 当前异常处理代码的位置 | 把 HardFault_Handler 自己当成肇事代码 |
| 异常入口 LR / EXC_RETURN | 硬件产生的返回模式、栈选择、帧类型编码 | 普通函数调用会覆盖 LR；必须明确捕获时点 |
| 帧中的 LR | 被中断上下文的普通链接寄存器 | 它不等于 EXC_RETURN，也不必是当前函数完整调用链 |
| 帧中的 PC | 被保存的上下文返回地址，具体 fault 类型影响解释 | 异步 bus fault、向量读取 fault 与精确访问 fault 不同 |

本文输入 `exc_return` 是已在正确入口时点捕获的编码，不是猜测“哪个 LR 看起来像全 F”。如果采集器把自己的 C 函数栈当成硬件入口帧，离线算术再正确也会解释错误对象。目标捕获代码需单独验证编译器 ABI、函数序言、优化、FPU 指令使用与所用栈；本章不提供未经编译的汇编作为已验证方案。

## EXC_RETURN、核心帧与浮点帧

本模型仅接受 CMSIS 对 Cortex-M4 定义的六个值：

| EXC_RETURN | 返回模式 | 原入口帧用的栈 | 帧类型 |
| --- | --- | --- | --- |
| `0xfffffff1` | Handler | MSP | 基本 |
| `0xfffffff9` | Thread | MSP | 基本 |
| `0xfffffffd` | Thread | PSP | 基本 |
| `0xffffffe1` | Handler | MSP | 浮点扩展 |
| `0xffffffe9` | Thread | MSP | 浮点扩展 |
| `0xffffffed` | Thread | PSP | 浮点扩展 |

不能因为当前 handler 在 MSP 上执行，就只查 MSP；被中断任务的硬件帧可能在 PSP。嵌套异常又可能保存 handler 上下文。这里白名单是教学解码器的范围选择，不是所有 Cortex-M 架构编码的完整列表。[CMSIS EXC_RETURN 定义](https://github.com/ARM-software/CMSIS_6/blob/v6.1.0/CMSIS/Core/Include/core_cm4.h)

核心帧从选中的、异常入口时点的帧 SP 起，依次是 R0、R1、R2、R3、R12、LR、PC、xPSR，即字偏移0～7。对于本文 M4 扩展帧，浮点空间位于这八个字之后；帧总长度为26字，基本帧为8字，额外对齐字另算。因此 core PC 仍是字偏移6。RTX 的固定 ARMv7-M SVC 入口先按 LR 的栈选择位取 MSP/PSP，再从偏移24字节取保存 PC；本章没有复用其实现。[Arm M4F 帧说明](https://documentation-service.arm.com/static/5f0352c0cafe527e86f5bc66)、[RTX SVC 入口](https://github.com/ARM-software/CMSIS-RTX/blob/v5.9.0/Source/GCC/irq_armv7m.S)

不要把额外浮点空间当成“所有 fault 都应跳过18字后取 R0”。这里使用的是本章固定 M4 布局；目标 debugger、RTOS 软件补存的 R4～R11/S16～S31 或包装结构也可能有自己的起点。必须先确认手中的指针是硬件入口帧 SP，还是软件扩展记录的基址。

### 对齐字不是 R0 前的第一个字段

堆栈 xPSR 的 bit9 记录硬件对齐调整。基本帧的额外字位于八字帧之后；扩展帧的额外字在整个扩展帧之后。不能把它插在 R0 前面，导致每个寄存器移位。恢复前 SP 的教学算式是：

```text
basic:    pre_sp = entry_sp + 4 * (8  + padding)
extended: pre_sp = entry_sp + 4 * (26 + padding)
padding = 1 if stacked_xPSR[9] else 0
```

这只是布局计算，不是在产品里执行异常返回。采集副本不足以覆盖完整帧或额外字时，本模型拒绝完整帧认证。目标栈边界应来自链接/任务栈管理的真实合同，不由“地址像 RAM”来猜。[CCR 与堆栈 PSR 对齐位](https://www.st.com/resource/en/programming_manual/pm0214-stm32-cortexm4-mcus-and-mpus-programming-manual-stmicroelectronics.pdf)

### 浮点 lazy 保存的另一个陷阱

EXC_RETURN 的扩展帧标记说明有浮点空间，不能单凭它断言 S0～S15/FPSCR 已经写入有效值。lazy 保存可先预留空间，FPCCR.LSPACT 记录待保存状态；嵌套情形还需要仔细解释保存来源。因此模型记录 LSPACT，但根本不输出浮点寄存器内容；遇到 LSPERR/MLSPERR 也保守拒绝自动认证。采集代码自己执行浮点指令还可能改变 lazy 保存过程，必须单独审查编译结果。[Arm lazy 保存机制](https://documentation-service.arm.com/static/5f0352c0cafe527e86f5bc66)

## 状态位、地址与可信程度

不要只打印 BFAR 就说“这个地址访问非法”。有效位决定该字段能否作为本次分析的线索；寄存器可能保留旧值，多个 fault 位也可能累积。采集时应保留原值与时点，清除状态要遵守目标的写1清除规则，不以读改写随意覆盖证据。

| 线索 | 本章的解释 / 策略 |
| --- | --- |
| HFSR.FORCED | fault 升级；继续查其他状态，不把“FORCED”当根因 |
| HFSR.VECTTBL | 异常向量读取相关故障；堆栈 PC 是被打断上下文，不能当向量读取指令 |
| CFSR.BFARVALID / MMARVALID | 分别控制 BFAR / MMFAR 的有效地址输出 |
| PRECISERR | 完整结构帧下的 PC 可作精确访问候选；仍需确认记录归属与累积状态 |
| IMPRECISERR | 不认证保存 PC 就是触发访问的指令 |
| STKERR / MSTKERR | 栈调整不等于各字都成功保存；本模型拒绝自动帧信任 |
| UNSTKERR / MUNSTKERR | 原返回栈可能仍存在，但需要专门恢复分析；本模型保守拒绝自动认证 |
| LSPERR / MLSPERR | 浮点延迟保存出现问题；不输出“完整浮点现场” |
| INVSTATE / INVPC / UNDEFINSTR / NOCP | 提供不同假设入口；不是统一叫“栈溢出” |
| UNALIGNED / DIVBYZERO | 还要核对指令及目标陷阱配置，不能一律从 C 源行猜原因 |

这些位的名称、位置与硬件语义按固定 PM0214/CMSIS 核对；“只要出现任何退栈/lazy错误就拒绝自动认证”是本实验主动采用的保守策略，并不是声称架构在所有这种情况下都彻底丢失了全部帧。[fault 状态与有效位](https://www.st.com/resource/en/programming_manual/pm0214-stm32-cortexm4-mcus-and-mpus-programming-manual-stmicroelectronics.pdf)、[CMSIS 位定义](https://github.com/ARM-software/CMSIS_6/blob/v6.1.0/CMSIS/Core/Include/core_cm4.h)

## 四段原创合成轨迹

### 轨迹 A：精确 bus fault 候选

输入 `EXC_RETURN=0xfffffffd`，PSP在已复制的8字副本内，xPSR.T=1，CFSR有 PRECISERR 和 BFARVALID；合成 PC=`0x08000200`，BFAR=`0x40000000`。模型输出结构接受、PSP/Thread/基本帧、有效 BFAR、精确 PC 候选。

下一步不是直接改 `0x08000200` 那行：先确认副本的固件身份和采集先后，再用匹配 ELF 解释 PC，查实际指令的有效地址、寄存器、访问宽度与外设时钟/权限。例子中的地址是合成数据，不是在任何具体 STM32 上执行过该访问。

### 轨迹 B：异步错误

把 CFSR 改成 IMPRECISERR，其他帧字不变。结构仍可解析，但精确 PC 候选为否，BFAR无有效位所以输出为空。不能通过重复运行解码器让这个 PC 变成根因。应扩大到之前的写操作、设备状态、最小复现与硬件追踪范围；怎样增加屏障或诊断可见性，要按目标架构和设备合同验证，不能声称一次 DSB 万能修复。

### 轨迹 C：堆栈错误

CFSR有 STKERR，即使副本看起来正好8字，模型拒绝自动认证，不取一个“像代码地址”的字充当 PC。应核对入口 MSP/PSP 边界、栈预算、保护区、嵌套和采集自身是否再次出错。增加栈可以是诊断实验，不是无需证据的结论。

### 轨迹 D：浮点扩展与 lazy 状态

使用 `EXC_RETURN=0xffffffed`，核心8字之后追加18字浮点/保留空间；若xPSR[9]=1，再追加对齐字。核心 PC 仍从偏移6取。模型验证完整副本范围、计算104或108字节帧大小；LSPACT=1时仅记录其状态，不解释那些合成浮点字。没有用主机浮点运算模拟 FPU。

## 原创离线解码实验

资源：[解码器](../../examples/python/cortex_m4_fault_snapshot.py)、[合成测试](../../tests/test_cortex_m4_fault_snapshot.py)。

```sh
python3 examples/python/cortex_m4_fault_snapshot.py
python3 -m unittest discover -s tests -p test_cortex_m4_fault_snapshot.py -v
```

短片段说明为什么先判 valid 位；它不读取地址所指内存：

```python
def valid_fault_addresses(cfsr, bfar, mmfar):
    return {
        "bfar": bfar if cfsr & (1 << 15) else None,
        "mmfar": mmfar if cfsr & (1 << 7) else None,
    }
```

完整文件更严格：数值为uint32，副本是不超过64字的tuple，区间由调用者显式声明；检查基址对齐、区间和地址溢出。再对白名单EXC_RETURN选择SP，检查六类帧风险位、完整布局及xPSR.T。返回 `structurally_accepted` 只表示合成副本满足本模型结构检查；`precise_pc_candidate` 也只是一种有条件的证据标签。模型既不探测真实RAM可读性，也不证明已复制的数据真实、同时或未被破坏。

实际16个测试方法通过，覆盖六种EXC_RETURN、MSP/PSP选择、基本/扩展帧和前缀、副本缺字、对齐字位置与缺失、地址有效位四种组合、六类帧风险、精确/异步/两者并存、FORCED、VECTTBL、T位不一致、非法编码、类型/范围/溢出拒绝与不修改输入。未执行任何真实fault或目标内存读取。

## 从离线线索到项目证据

目标采集与符号化是另外两层验收，不能用16个主机方法代替：

1. 构建记录：固件版本、构建身份、目标芯片/核心、编译选项、ELF/map和链接区域。
2. 捕获合同：入口时点的EXC_RETURN/MSP/PSP；状态寄存器的保存顺序；哪个栈和内存区可安全复制；长度上限；二次fault的退路。
3. 留存合同：固定大小记录、版本/长度/校验、写入完成标记，复位原因和断电/重启行为。不要在异常路径无限printf、申请堆内存、等待一个可能已损坏的锁。
4. 符号检查：匹配ELF，查看候选PC周围反汇编与访问寄存器；优化内联和一条源行多指令必须考虑。不从几个“像返回地址”的字自动重建完整调用链。
5. 假设验证：每个假设写出能反驳它的观察。例如“栈溢出”需边界与破坏证据；“野指针”需来源/寿命轨迹；“DMA覆盖”需拥有权、区域与停止状态。
6. 目标验收：受控故障、缺失/损坏记录、嵌套、浮点lazy、采集二次异常、启动读取与构建错配。这些本轮未运行。

Linux崩溃证据链另读 [Linux故障与事后证据](linux-postmortem-evidence.md)；对象寿命另读 [C++发布与复用合同](../10-c-cpp/publication-ownership-contracts.md)，DMA关闭路径另读 [DMA取消与移除合同](../80-linux-drivers/dma-lifecycle-contracts.md)。错误类型可能相连，但不同架构的寄存器和接口不能合并成一种解码器。

## 递进追问链

| 追问 | 答案要落实到什么 |
| --- | --- |
| “HardFault就是栈溢出？” | 区分多类fault、升级与证据，不先认定原因 |
| handler用MSP，为何看PSP？ | EXC_RETURN说明原硬件入口帧的栈；实际C函数栈另算 |
| LR=`0xfffffffd`说明什么？ | 本M4范围：Thread/PSP/基本帧，且必须是入口捕获值 |
| FPU开启后PC要跳18字吗？ | 本文M4核心仍在SP+0x18；解释完整扩展帧起点和软件保存区别 |
| BFAR=`0xdeadbeef`就是野指针吗？ | 先检查BFARVALID、采集时点与状态累积 |
| xPSR[9]怎么用？ | 整个帧之后的一字对齐补偿，检查副本长度 |
| 精确/异步bus fault怎样改调查？ | 前者提供指令候选，后者扩大到先前操作；都要构建身份 |
| 退栈fault是不是所有帧都没了？ | 架构可能保留原栈；模型只是拒绝自动认证，需要专门恢复 |
| 栈帧成功解析，能自动继续跑吗？ | 不能；资源/外设/同步状态和产品安全要求尚未证明 |

基础：算对栈选择、核心偏移与地址有效位。进阶：明确帧类型、对齐、lazy/嵌套和不可信状态。项目：给出板级采集、准确构建与可反驳的根因实验。本轮只完成离线主机层。

## 面试线索与整理边界

现有面试线索 [大疆volatile与HardFault提问](https://ac.nowcoder.com/discuss/1549653) 已在此前来源记录中，本轮不重复计作新样本；公开题单 [embedded-interview](https://github.com/Sering-Hong/embedded-interview) 用于核对提问范围，不作为寄存器答案权威。本章章文、合成输入、模型与追问均为原创；平台题目不能替代固定官方资料，也不能证明目标验证已完成。
