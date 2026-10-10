# 公开面经 → 原理 → 故障场景

先复习基础，再用面经练项目追问和异常路径。本模块包含17份新增完整自报页面、7张原创场景卡；与120核心主题、Hot100的100+1题分别计数，不把面经变成公司题库。核对日：2026-10-10。

## 先选主方向

- MCU / RTOS：IC01队列过载 → IC02关闭 → IC03 DMA/cache → IC05/IC06升级恢复。
- Linux BSP / 驱动：IC04 DMA所有权 → IC07 remove与fd寿命。先有C/资源管理和Linux接口基础。
- 第一次实习：先选[来源目录](source-catalog.md)中明确实习/初级的EN01–EN07、EN16–EN18之一，按项目→原理→异常→验证练表达。EN07含混合岗位，使用时读分段边界。

## 七个原创故障场景

- [IC01 · UART中断把队列塞满，偶发丢命令](ic01.md)：UART / ISR / 背压
- [IC02 · 停止采集后删队列，下一次中断 HardFault](ic02.md)：RTOS关闭 / 对象寿命
- [IC03 · Cortex-M7 DMA接收结束，CPU仍读到旧帧](ic03.md)：MCU DMA / cache
- [IC04 · Linux DMA收发环，指针地址与缓存所有权混用](ic04.md)：Linux DMA / 所有权
- [IC05 · OTA在擦除、复制或状态写入时断电](ic05.md)：OTA / 掉电恢复
- [IC06 · 新固件能启动，却在一分钟后故障，如何确认升级](ic06.md)：OTA / 试运行确认
- [IC07 · Linux设备remove后，打开的fd和异步worker仍访问旧对象](ic07.md)：Linux驱动 / remove

每卡包含机制解释、起手答法、追问答案、误区、版本边界和验收。具体故障/状态机是原创延伸，不冒充面经作者原题。FreeRTOS V11.1.0 ARM_CM4F、CMSIS5.9.0、Linux6.6、MCUboot v2.1.0与Zephyr3.7只是注明的技术依据，换目标需复核。

## 面经主题如何回到讲义

| 主题 | 先修讲义 | 场景 |
|---|---|---|
| UART / ISR / 背压 | [MC04](../40-mcu/mc04.md)、[RT05](../50-rtos/rt05.md)、[RT06](../50-rtos/rt06.md)、[RT09](../50-rtos/rt09.md) | [IC01](ic01.md) |
| RTOS关闭 / 对象寿命 | [RT10](../50-rtos/rt10.md)、[RT06](../50-rtos/rt06.md)、[MC05](../40-mcu/mc05.md) | [IC02](ic02.md) |
| MCU DMA / cache | [MC05](../40-mcu/mc05.md)、[MC06](../40-mcu/mc06.md) | [IC03](ic03.md) |
| Linux DMA / 所有权 | [DV05](../80-linux-drivers/dv05.md)、[DV04](../80-linux-drivers/dv04.md) | [IC04](ic04.md) |
| OTA / 掉电恢复 | [MC10](../40-mcu/mc10.md)、[BT04](../90-boot-rootfs/bt04.md)、[BT05](../90-boot-rootfs/bt05.md) | [IC05](ic05.md) |
| OTA / 试运行确认 | [MC09](../40-mcu/mc09.md)、[BT04](../90-boot-rootfs/bt04.md) | [IC06](ic06.md) |
| Linux驱动 / remove | [DV01](../80-linux-drivers/dv01.md)、[DV07](../80-linux-drivers/dv07.md)、[DV10](../80-linux-drivers/dv10.md) | [IC07](ic07.md) |

C/C++指针与寿命回到[C01讲义](../10-c-cpp/c01.md)和[C02讲义](../10-c-cpp/c02.md)，编译/启动回到[AR01](../30-architecture/ar01.md)与[AR03](../30-architecture/ar03.md)，Linux内存/并发回到[LX16](../70-linux-apps/lx16.md)和[LX04](../70-linux-apps/lx04.md)。算法回到[已有Hot100](../80-hot100-cpp/README.md)，不重复扩写题解。

## 一轮30–60分钟练习

1. 读一份与目标岗位匹配的自报面经，挑一个能回到本人项目的主题。
2. 先不看解答，说30秒起手回答，再画拥有权/时序/失败路径。
3. 读基础与原创场景，补一条追问；不会的寄存器指出要查的目标手册。
4. 手推或实现最小状态模型，记下尚未运行的检查，避免虚构板测或生产指标。
5. 记录一个验证结果或知识缺口，下次用变式复述。时间是学习建议。

## 来源与验证边界

- [17份完整页面、试读与GitHub日志](source-catalog.md)：公司、岗位、轮次、时间仅按作者自述；本人身份和面试发生未独立核验。
- 17份新增完整页面按发布/编辑年为2021–2025。本轮新增2026年完整页面为0；[原有六条](../../sources/interview-sample.json)另保留，不据旧样本推断当前招聘规则。
- 小红书本轮原始正文/日期/经历可核验页面为0。GitHub无明确发表日的日志与公开试读不计完整样本。
- 页面日期、最后编辑和面试日期分开，不由月日、毕业届数或仓库更新推断年份。
- 公开样本有搜索、平台和自我选择偏差，不计算命中率、行业频率或录用概率。
- 七卡练习均为设计，未运行RTOS、板级DMA、Flash掉电、OTA或内核模块/KASAN；官方文档支持机制边界，不能代替实测。
- 本仓库仅给原创摘要、原创教学与公开链接，不附原帖长篇正文、图片题库或付费全文。

[结构化场景映射](manifest.json) · [结构化来源](../../sources/interview-supplement.json) · [完整学习路线](../00-guide/learning-path.md) · [模拟面试复盘](../110-projects/pj04.md)
