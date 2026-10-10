# 知道已经做了什么，也知道还没有做什么

## 已建成的课程

原创核心题按语言/内存、STL、构建/架构、STM32、FreeRTOS、协议网络、Linux应用、驱动、启动/rootfs、调试、项目划分。每题有机制、例子、失败边界和练习。题数由core-question-manifest与Hot100 manifest生成，不手填宣传数字。

## 课程范围

课程按主题组织，完整题号、讲义路径与公开官方来源由课程清单维护。尚未展开的专题包括WebSocket与socket专门对比、GD32/STM32启动文件逐版本差异、Linux常用指令参考、传统daemon完整实现与内核调度器深专题。已有合并主题不代表每个专门方向都已完成独立实验。

## 验证边界

有限主机测试覆盖C逻辑、十进制边界解析、C++资源/队列/安全捕获、Linux进程与FD生命期和算法实现。未做真实MCU、电气/时序、FreeRTOS移植、Linux驱动加载、BSP/QEMU启动、功耗、Flash耐久和真实OTA。ASan/UBSan通过仅限执行路径；本容器LeakSanitizer被ptrace限制阻断，显式禁用后运行的检查不能称泄漏测试通过。

## 继续深入的路线

USB、Ethernet PHY/lwIP实际移植、MQTT/CoAP、CAN位时序/物理层、嵌入式GUI、DSP/模型量化部署、Linux驱动完整工程与长期板级故障注入尚未展开。选定芯片/目标内核后补版本化实验，优先关闭真实项目的一条链路，避免只扩充名词数量。

## FreeRTOS源码扩展

[源码导读](../50-rtos/source-reading.md)及六篇相邻扩展共7篇，分别说明链表、任务、队列、事件组、timer daemon和历史co-routines。现代主线固定V11.1.0单核GCC ARM_CM4F，历史章节固定V9.0.0并独立标注许可。它们补充已有RT01–RT10，不增加核心题数量。公开源码核读和原创手推不代表主机模型、内核、模拟器或板级实验已经执行；SMP、其他port和实际负载须重新核对。
