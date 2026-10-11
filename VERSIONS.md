# 版本与适用范围

核查日期：2026-10-10。

- 精确数值span解析要求8位字节与ASCII兼容执行字符集，编译期检查；不声明EBCDIC目标已支持。
- C语言教学/主机代码：C11，N1570为公开委员会草案；不是最新C正式标准。
- C++可运行代码：C++17。N4659为公开工作草案，eel.is为滚动草案；出现后续版本规则时必须单独标注。
- 主机工具：本次环境GCC/G++14.2.0、Python3.12.14、Node24.19.0；页面构建不需要下载npm依赖。
- MCU：通用Cortex-M机制，M4细节引用PM0214；具体STM32外设以F4 RM0090作查证入口。它的官方链接已核验，全文受工具体积限制未完整读入。不能宣称某块开发板已跑通。
- M7 cache/DMA：F7/H7 AN4839单独进阶，不暗示F407有M7 D-cache。bxCAN与FDCAN不能混讲。
- FreeRTOS：常见单核Cortex-M语境，使用前固定项目Kernel版本/port/config。SMP、时间片、栈单位和中断上限按配置核查；本仓库未编译或运行Kernel。
- 额外主机练习：真实C调用方按C11编译、opaque实现按C++17 -fno-exceptions编译；Linux生命周期依赖/proc/self/exe与/tmp，只运行自己创建的进程/文件。
- Linux：用户API按目标libc/kernel核对；滚动docs.kernel.org不是稳定内核ABI。驱动例子先固定内核commit/config，PREEMPT_RT改变部分锁语义。主机测试不能代替模块加载/板级bring-up。
- U-Boot：SPL/TPL/VPL为平台可选阶段，不是固定启动链。
- Buildroot：核查时官方manual标2026.08，滚动链接可变，实际项目保存release/commit与defconfig。
- 网络：TCP RFC9293(2022)、UDP RFC768、TLS1.3 RFC8446；应用协议明确长度/安全/重试，不靠传输层包边界。
- 静态渲染器：Marked17.0.5 MIT，第三方许可证保存在site/vendor。无CDN、统计、后台接口、登录或上传阅读记录。

测试仅在有限输入和本次环境提供证据。文中未测项目均写明，不把模型、QEMU或主机结果推定为真实硬件通过。

## FreeRTOS 源码扩展的固定基线

- 六篇现代源码讲义：FreeRTOS-Kernel V11.1.0，commit `dbf70559b27d39c1fdb68dfb9a32140b6a6777a0`，单核GCC `portable/GCC/ARM_CM4F`；tick宽度、抢占/时间片、timer队列和可选功能以各章明确的配置为准。[固定源码与MIT许可](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0/LICENSE.md)
- 历史co-routines选读：FreeRTOS V9.0.0，commit `165c24cba94e3d3f386c3b77d3bb0f4a524d8511`；GPLv2加FreeRTOS exception，与现代MIT边界分别记录。[历史许可](https://github.com/FreeRTOS/FreeRTOS/blob/165c24cba94e3d3f386c3b77d3bb0f4a524d8511/FreeRTOS/License/license.txt)
- 源码核读、原创伪代码和手推时序不是运行记录；这些章节没有编译或运行主机模型、内核、模拟器及板级实验。其他版本、SMP、MPU包装、厂商修改内核和其他port须重新核对。

## 2026-10-11 · 协议三章的固定教学基线

- [I²C时序与恢复](docs/60-protocols/i2c-timing-recovery.md)：NXP UM10204 Rev.7.0（2021-10-01），只核对章内相关时序、上拉和bus-clear段落；原创策略和规范原句分别说明。
- [SPI逐边沿与事务](docs/60-protocols/spi-edges-spidev.md)：官方Linux v6.6 UAPI与spidev文档。主机C代码实际编译于本机系统头文件并注入mock ioctl，没有编译/运行Linux v6.6内核或访问真实设备。
- [经典Reno与RTO](docs/60-protocols/tcp-reno-rto.md)：RFC9293、5681和6298；采用章内显式限定的单缺口、无SACK、有限事件教学模型，不能代表Linux默认拥塞算法或真实网络。

这三章的规范核对、合成状态/边沿和主机运行范围单独记录，不替代板级电气、目标内核或生产网络验证。未恢复既有受限或未读原文，不据此宣称全部来源已完整覆盖。
