# 题目索引

原创核心：120主题。官方Hot100：100题。额外公开练习：1题。

优先级是课程依赖建议，不是面试考频。题目按manifest生成；实现是否实际测试以验证报告为准。

## C / C++ 与内存 · 22主题

- [C01 · 数组、指针和长度为什么必须一起设计？](docs/10-c-cpp/c01.md) · P0
- [C02 · 栈、堆、静态存储与对象生命周期怎么区分？](docs/10-c-cpp/c02.md) · P0
- [C03 · const 能保证什么，不能保证什么？](docs/10-c-cpp/c03.md) · P0
- [C04 · volatile、atomic 与内存屏障怎样分工？](docs/10-c-cpp/c04.md) · P0
- [C05 · 未定义行为、未指定行为与实现定义如何回答？](docs/10-c-cpp/c05.md) · P0
- [C06 · 结构体对齐、padding 与网络序列化有什么关系？](docs/10-c-cpp/c06.md) · P0
- [C07 · malloc/free 与嵌入式内存分配如何取舍？](docs/10-c-cpp/c07.md) · P0
- [C08 · C++ RAII、unique_ptr 与错误路径有什么价值？](docs/10-c-cpp/c08.md) · P0
- [C09 · 引用、拷贝、移动和完美转发如何区分？](docs/10-c-cpp/c09.md) · P0
- [C10 · 回调函数、函数指针和上下文对象怎么设计？](docs/10-c-cpp/c10.md) · P0
- [C11 · 虚析构、虚函数与对象切片有什么区别？](docs/10-c-cpp/c11.md) · P0
- [C12 · 整型提升、位运算与移位如何写得可移植？](docs/10-c-cpp/c12.md) · P0
- [C13 · static、extern、声明与定义怎样区分？](docs/10-c-cpp/c13.md) · P0
- [C14 · memcpy、memmove 与字符串 API 怎样避免越界？](docs/10-c-cpp/c14.md) · P0
- [C15 · class/struct、访问控制与 const 成员函数的真实边界？](docs/10-c-cpp/c15.md) · P0
- [C16 · shared_ptr、weak_ptr 与循环引用如何判断？](docs/10-c-cpp/c16.md) · P1
- [C17 · 宏、inline 与 constexpr 怎样避免重复求值和控制流陷阱？](docs/10-c-cpp/c17.md) · P0
- [C18 · atoi、strtol 与有界数值解析怎样定义失败合同？](docs/10-c-cpp/c18.md) · P0
- [C19 · 成员初始化顺序与 Rule Zero/Five 怎样维护对象不变量？](docs/10-c-cpp/c19.md) · P0
- [C20 · lambda 捕获怎样跨越异步回调的生命周期？](docs/10-c-cpp/c20.md) · P0
- [C21 · extern "C" 与 opaque handle 怎样建立可靠的 C/C++ 边界？](docs/10-c-cpp/c21.md) · P0
- [C22 · 重载、覆盖、名字隐藏与默认参数怎样决定一次调用？](docs/10-c-cpp/c22.md) · P1

## STL 与数据结构 · 7主题

- [ST01 · vector、array、list 的选择依据是什么？](docs/20-stl/st01.md) · P0
- [ST02 · 迭代器失效为什么比复杂度更容易出 bug？](docs/20-stl/st02.md) · P0
- [ST03 · map、unordered_map 与哈希退化怎么回答？](docs/20-stl/st03.md) · P1
- [ST04 · string、string_view 与二进制数据有哪些坑？](docs/20-stl/st04.md) · P0
- [ST05 · 排序的比较器必须满足什么？](docs/20-stl/st05.md) · P1
- [ST06 · 复杂度、内存上界与实时性怎样一起讲？](docs/20-stl/st06.md) · P0
- [ST07 · deque、stack、queue 和 priority_queue 应按什么合同选择？](docs/20-stl/st07.md) · P1

## 架构、编译与链接 · 9主题

- [AR01 · 预处理、编译、汇编、链接各负责什么？](docs/30-architecture/ar01.md) · P0
- [AR02 · 链接脚本、Flash/RAM 与 VMA/LMA 怎么理解？](docs/30-architecture/ar02.md) · P0
- [AR03 · Cortex-M 从复位到 main 发生什么？](docs/30-architecture/ar03.md) · P0
- [AR04 · 中断优先级与抢占优先级怎样避免混淆？](docs/30-architecture/ar04.md) · P0
- [AR05 · ABI、调用约定与栈对齐为什么会影响固件？](docs/30-architecture/ar05.md) · P1
- [AR06 · 交叉编译怎样避免误链接主机库？](docs/30-architecture/ar06.md) · P0
- [AR07 · MSP、PSP、SVC 和 PendSV 如何参与任务切换？](docs/30-architecture/ar07.md) · P0
- [AR08 · Harvard/von Neumann、MCU/MPU 与实时性如何区分？](docs/30-architecture/ar08.md) · P0
- [AR09 · Make 的依赖图、头文件追踪与增量构建怎样验证？](docs/30-architecture/ar09.md) · P1

## STM32 与外设 · 10主题

- [MC01 · GPIO 推挽、开漏、上下拉怎么选？](docs/40-mcu/mc01.md) · P0
- [MC02 · 定时器频率、PWM 与时钟倍频怎么计算？](docs/40-mcu/mc02.md) · P0
- [MC03 · 输入捕获测频率和编码器有哪些边界？](docs/40-mcu/mc03.md) · P0
- [MC04 · UART 接收为什么必须设计帧边界？](docs/40-mcu/mc04.md) · P0
- [MC05 · DMA 什么时候比中断逐字节更合适？](docs/40-mcu/mc05.md) · P0
- [MC06 · Cortex-M7 DMA 与 cache 一致性怎么处理？](docs/40-mcu/mc06.md) · P0
- [MC07 · ADC 精度、采样时间与滤波怎么区分？](docs/40-mcu/mc07.md) · P1
- [MC08 · 按键消抖和非阻塞状态机怎样设计？](docs/40-mcu/mc08.md) · P0
- [MC09 · 看门狗与低功耗为何不能孤立设计？](docs/40-mcu/mc09.md) · P1
- [MC10 · Flash 擦写和掉电安全存储怎么做？](docs/40-mcu/mc10.md) · P1

## FreeRTOS 与并发 · 10主题

- [RT01 · 任务状态、抢占和时间片怎么区分？](docs/50-rtos/rt01.md) · P0
- [RT02 · vTaskDelay 与 vTaskDelayUntil 的周期差异？](docs/50-rtos/rt02.md) · P0
- [RT03 · 互斥量、二值信号量和计数信号量怎么选？](docs/50-rtos/rt03.md) · P0
- [RT04 · 优先级反转、继承与死锁怎么解释？](docs/50-rtos/rt04.md) · P0
- [RT05 · 队列、任务通知和事件组的语义差别？](docs/50-rtos/rt05.md) · P0
- [RT06 · FromISR API 与中断优先级上限怎么配合？](docs/50-rtos/rt06.md) · P0
- [RT07 · 临界区、调度器挂起和中断屏蔽有什么不同？](docs/50-rtos/rt07.md) · P0
- [RT08 · 任务栈、heap_1..heap_5 与内存预算怎么看？](docs/50-rtos/rt08.md) · P0
- [RT09 · SPSC 环形缓冲怎样避免看似正确的竞态？](docs/50-rtos/rt09.md) · P0
- [RT10 · 任务创建、参数生命周期与软件定时器怎么设计？](docs/50-rtos/rt10.md) · P0

## 协议与网络 · 13主题

- [PR01 · I²C 起止、ACK、仲裁和时钟拉伸怎么理解？](docs/60-protocols/pr01.md) · P0
- [PR02 · SPI 模式、片选与事务边界怎么核对？](docs/60-protocols/pr02.md) · P0
- [PR03 · RS485 和 Modbus RTU 不是同一层东西？](docs/60-protocols/pr03.md) · P1
- [PR04 · CAN 仲裁、标识符和错误处理怎样建立主线？](docs/60-protocols/pr04.md) · P2
- [PR05 · OSI/TCP-IP 分层对嵌入式排障有何用？](docs/60-protocols/pr05.md) · P0
- [PR06 · TCP 为什么会半包和粘包，怎样正确接收？](docs/60-protocols/pr06.md) · P0
- [PR07 · TCP 建连、关闭和 TIME_WAIT 怎么讲？](docs/60-protocols/pr07.md) · P0
- [PR08 · TCP/UDP、超时重试和幂等怎样一起选？](docs/60-protocols/pr08.md) · P0
- [PR09 · CRC、checksum、MAC 和签名怎么区分？](docs/60-protocols/pr09.md) · P1
- [PR10 · IP、MAC、子网掩码与 ARP 怎样串起来？](docs/60-protocols/pr10.md) · P0
- [PR11 · DNS、HTTP、HTTPS 与 TLS 握手怎样避免混说？](docs/60-protocols/pr11.md) · P0
- [PR12 · TCP 流量控制与拥塞控制有什么不同？](docs/60-protocols/pr12.md) · P0
- [PR13 · HTTP 的方法语义、消息定界和重试怎样写对？](docs/60-protocols/pr13.md) · P0

## Linux 应用 · 18主题

- [LX01 · 进程、线程和虚拟地址空间怎么区分？](docs/70-linux-apps/lx01.md) · P0
- [LX02 · read/write 的短读、短写与 EINTR 如何处理？](docs/70-linux-apps/lx02.md) · P0
- [LX03 · 阻塞、非阻塞、select/poll/epoll 如何建立模型？](docs/70-linux-apps/lx03.md) · P0
- [LX04 · pthread 互斥量与条件变量如何正确配合？](docs/70-linux-apps/lx04.md) · P0
- [LX05 · mmap 共享内存、文件映射和持久化的边界？](docs/70-linux-apps/lx05.md) · P0
- [LX06 · 信号处理函数为什么不能随便打印和加锁？](docs/70-linux-apps/lx06.md) · P1
- [LX07 · IPC 的 pipe、socket 和共享内存怎么选？](docs/70-linux-apps/lx07.md) · P0
- [LX08 · 从应用性能问题到 CPU/内存/I/O 证据链？](docs/70-linux-apps/lx08.md) · P1
- [LX09 · join、detach、退出与资源回收怎么设计？](docs/70-linux-apps/lx09.md) · P0
- [LX10 · timerfd 和绝对时刻周期任务怎么保证边界？](docs/70-linux-apps/lx10.md) · P0
- [LX11 · 系统调用、库函数与用户态/内核态有何不同？](docs/70-linux-apps/lx11.md) · P0
- [LX12 · Linux GPIO/UART/SPI/I²C 应用应走什么接口？](docs/70-linux-apps/lx12.md) · P1
- [LX13 · fork、exec、wait 与僵尸回收怎样形成完整生命周期？](docs/70-linux-apps/lx13.md) · P0
- [LX14 · fd、打开文件描述、inode 与 unlink 的生命周期怎么区分？](docs/70-linux-apps/lx14.md) · P0
- [LX15 · 教材调度算法、Linux 公平策略和实时上限怎样分层回答？](docs/70-linux-apps/lx15.md) · P1
- [LX16 · 页表、TLB、缺页和 COW 如何解释虚拟内存？](docs/70-linux-apps/lx16.md) · P0
- [LX17 · 有界线程池怎样设计背压、拥有权和停止协议？](docs/70-linux-apps/lx17.md) · P1
- [LX18 · select、poll、epoll 的资源上限与公平预算怎么验证？](docs/70-linux-apps/lx18.md) · P0

## Linux 驱动 · 12主题

- [DV01 · 字符设备、file_operations 和用户态 fd 如何连起来？](docs/80-linux-drivers/dv01.md) · P0
- [DV02 · 为什么不能直接解引用用户指针？](docs/80-linux-drivers/dv02.md) · P0
- [DV03 · platform 驱动、设备树与 probe 的关系？](docs/80-linux-drivers/dv03.md) · P0
- [DV04 · MMIO、ioremap 与寄存器读改写有什么风险？](docs/80-linux-drivers/dv04.md) · P0
- [DV05 · Linux DMA mapping 与裸机 DMA 为什么不同？](docs/80-linux-drivers/dv05.md) · P0
- [DV06 · 内核 mutex、spinlock 与中断上下文怎么选？](docs/80-linux-drivers/dv06.md) · P0
- [DV07 · 中断下半部、workqueue 与线程化 IRQ 怎么选？](docs/80-linux-drivers/dv07.md) · P0
- [DV08 · ioctl、sysfs、poll 的接口设计原则？](docs/80-linux-drivers/dv08.md) · P0
- [DV09 · kmalloc、vmalloc 与 GFP 标志怎么选？](docs/80-linux-drivers/dv09.md) · P0
- [DV10 · 内核模块加载、probe、移除与版本匹配怎么验证？](docs/80-linux-drivers/dv10.md) · P0
- [DV11 · RCU 为什么允许并发读写，旧对象什么时候能释放？](docs/80-linux-drivers/dv11.md) · P1
- [DV12 · 设备号、cdev、sysfs、devtmpfs 与 /dev 怎样连起来？](docs/80-linux-drivers/dv12.md) · P0

## 启动与根文件系统 · 8主题

- [BT01 · Boot ROM、SPL、U-Boot、kernel、init 的责任链？](docs/90-boot-rootfs/bt01.md) · P0
- [BT02 · kernel image、DTB、initramfs 与 rootfs 的区别？](docs/90-boot-rootfs/bt02.md) · P0
- [BT03 · Buildroot、BusyBox 与可复现系统如何搭建？](docs/90-boot-rootfs/bt03.md) · P0
- [BT04 · OTA 的 A/B、回滚与掉电安全如何联动？](docs/90-boot-rootfs/bt04.md) · P1
- [BT05 · Flash 分区、环境变量与救砖如何规划？](docs/90-boot-rootfs/bt05.md) · P2
- [BT06 · initramfs、initrd、文件系统和 PID1 怎么区分？](docs/90-boot-rootfs/bt06.md) · P0
- [BT07 · 交叉编译 Linux 内核后怎样验证目标部署契约？](docs/90-boot-rootfs/bt07.md) · P1
- [BT08 · U-Boot 的 bootm、booti 与内核入口契约怎样核对？](docs/90-boot-rootfs/bt08.md) · P0

## 调试与工程质量 · 6主题

- [DB01 · HardFault 怎么从寄存器走到源码？](docs/100-debugging/db01.md) · P0
- [DB02 · GDB、SWD/JTAG、断点与观察点怎么用？](docs/100-debugging/db02.md) · P0
- [DB03 · 逻辑分析仪、示波器与软件日志各能证明什么？](docs/100-debugging/db03.md) · P0
- [DB04 · 主机单元测试、sanitizer 与板测如何分层？](docs/100-debugging/db04.md) · P0
- [DB05 · 故障注入、背压和恢复为何比正常 Demo 重要？](docs/100-debugging/db05.md) · P0
- [DB06 · 版本、CI、勘误与公开项目该保存什么？](docs/100-debugging/db06.md) · P0

## 项目与面试表达 · 5主题

- [PJ01 · 怎样做一个可答辩的 MCU 数据采集项目？](docs/110-projects/pj01.md) · P0
- [PJ02 · 怎样做一个 Linux 设备网关项目？](docs/110-projects/pj02.md) · P0
- [PJ03 · 面试项目介绍怎样从功能讲到工程证据？](docs/110-projects/pj03.md) · P0
- [PJ04 · 一场模拟面试如何评分与复盘？](docs/110-projects/pj04.md) · P0
- [PJ05 · 面向 EE 背景怎样连接硬件优势与软件短板？](docs/110-projects/pj05.md) · P1

## 公开面经与原创故障场景 · 7卡

17份新增完整自报页面，试读和无日期GitHub日志另列；场景不计入120核心主题或Hot100。

- [来源日期、岗位与学习导航](docs/120-interviews/README.md)
- [公开自报目录](docs/120-interviews/source-catalog.md)
- [IC01 · UART中断把队列塞满，偶发丢命令](docs/120-interviews/ic01.md)
- [IC02 · 停止采集后删队列，下一次中断 HardFault](docs/120-interviews/ic02.md)
- [IC03 · Cortex-M7 DMA接收结束，CPU仍读到旧帧](docs/120-interviews/ic03.md)
- [IC04 · Linux DMA收发环，指针地址与缓存所有权混用](docs/120-interviews/ic04.md)
- [IC05 · OTA在擦除、复制或状态写入时断电](docs/120-interviews/ic05.md)
- [IC06 · 新固件能启动，却在一分钟后故障，如何确认升级](docs/120-interviews/ic06.md)
- [IC07 · Linux设备remove后，打开的fd和异步worker仍访问旧对象](docs/120-interviews/ic07.md)

## FreeRTOS源码扩展 · 7篇

基础RT01–RT10验收后按下列顺序阅读；扩展讲义不计入120核心或官方Hot100，历史co-routines为P2选读。

- [FreeRTOS 源码导读 · 先固定版本，再追状态变化](docs/50-rtos/source-reading.md) · P1 · 源码导读 / 版本配置
- [FreeRTOS 内核链表 · 节点归属、排序与轮转](docs/50-rtos/kernel-lists.md) · P1 · 内核链表 / 不变量
- [FreeRTOS 任务生命周期 · 从 TCB 到阻塞与唤醒](docs/50-rtos/kernel-task-lifecycle.md) · P1 · 任务生命周期 / 状态迁移
- [FreeRTOS 队列等待 · 复制、解阻塞与超时复查](docs/50-rtos/kernel-queue-waits.md) · P1 · 队列等待 / 超时
- [源码补充 · 事件组的条件判断、快照与延迟处理](docs/50-rtos/kernel-event-groups.md) · P1 · 事件组 / 条件快照
- [软件定时器与共享 daemon 的命令和寿命](docs/50-rtos/kernel-timer-daemon.md) · P1 · 软件定时器 / daemon寿命
- [历史选读 · FreeRTOS V9.0.0 co-routines 如何保存状态？](docs/50-rtos/historical-co-routines.md) · P2 · 历史选读 / co-routines

## Hot100 · C++17

- [学习路线、完整题单与测试](docs/80-hot100-cpp/README.md)
- [源码与编译](algorithms/cpp/README.md)

### 哈希 · 3题

- [LC1 · 两数之和](docs/80-hot100-cpp/problems/0001-two-sum.md) · EASY
- [LC49 · 字母异位词分组](docs/80-hot100-cpp/problems/0049-group-anagrams.md) · MEDIUM
- [LC128 · 最长连续序列](docs/80-hot100-cpp/problems/0128-longest-consecutive-sequence.md) · MEDIUM

### 双指针 · 4题

- [LC283 · 移动零](docs/80-hot100-cpp/problems/0283-move-zeroes.md) · EASY
- [LC11 · 盛最多水的容器](docs/80-hot100-cpp/problems/0011-container-with-most-water.md) · MEDIUM
- [LC15 · 三数之和](docs/80-hot100-cpp/problems/0015-3sum.md) · MEDIUM
- [LC42 · 接雨水](docs/80-hot100-cpp/problems/0042-trapping-rain-water.md) · HARD

### 滑动窗口 · 2题

- [LC3 · 无重复字符的最长子串](docs/80-hot100-cpp/problems/0003-longest-substring-without-repeating-characters.md) · MEDIUM
- [LC438 · 找到字符串中所有字母异位词](docs/80-hot100-cpp/problems/0438-find-all-anagrams-in-a-string.md) · MEDIUM

### 子串 · 3题

- [LC560 · 和为 K 的子数组](docs/80-hot100-cpp/problems/0560-subarray-sum-equals-k.md) · MEDIUM
- [LC239 · 滑动窗口最大值](docs/80-hot100-cpp/problems/0239-sliding-window-maximum.md) · HARD
- [LC76 · 最小覆盖子串](docs/80-hot100-cpp/problems/0076-minimum-window-substring.md) · HARD

### 普通数组 · 5题

- [LC53 · 最大子数组和](docs/80-hot100-cpp/problems/0053-maximum-subarray.md) · MEDIUM
- [LC56 · 合并区间](docs/80-hot100-cpp/problems/0056-merge-intervals.md) · MEDIUM
- [LC189 · 轮转数组](docs/80-hot100-cpp/problems/0189-rotate-array.md) · MEDIUM
- [LC238 · 除了自身以外数组的乘积](docs/80-hot100-cpp/problems/0238-product-of-array-except-self.md) · MEDIUM
- [LC41 · 缺失的第一个正数](docs/80-hot100-cpp/problems/0041-first-missing-positive.md) · HARD

### 矩阵 · 4题

- [LC73 · 矩阵置零](docs/80-hot100-cpp/problems/0073-set-matrix-zeroes.md) · MEDIUM
- [LC54 · 螺旋矩阵](docs/80-hot100-cpp/problems/0054-spiral-matrix.md) · MEDIUM
- [LC48 · 旋转图像](docs/80-hot100-cpp/problems/0048-rotate-image.md) · MEDIUM
- [LC240 · 搜索二维矩阵 II](docs/80-hot100-cpp/problems/0240-search-a-2d-matrix-ii.md) · MEDIUM

### 链表 · 14题

- [LC160 · 相交链表](docs/80-hot100-cpp/problems/0160-intersection-of-two-linked-lists.md) · EASY
- [LC206 · 反转链表](docs/80-hot100-cpp/problems/0206-reverse-linked-list.md) · EASY
- [LC234 · 回文链表](docs/80-hot100-cpp/problems/0234-palindrome-linked-list.md) · EASY
- [LC141 · 环形链表](docs/80-hot100-cpp/problems/0141-linked-list-cycle.md) · EASY
- [LC142 · 环形链表 II](docs/80-hot100-cpp/problems/0142-linked-list-cycle-ii.md) · MEDIUM
- [LC21 · 合并两个有序链表](docs/80-hot100-cpp/problems/0021-merge-two-sorted-lists.md) · EASY
- [LC2 · 两数相加](docs/80-hot100-cpp/problems/0002-add-two-numbers.md) · MEDIUM
- [LC19 · 删除链表的倒数第 N 个结点](docs/80-hot100-cpp/problems/0019-remove-nth-node-from-end-of-list.md) · MEDIUM
- [LC24 · 两两交换链表中的节点](docs/80-hot100-cpp/problems/0024-swap-nodes-in-pairs.md) · MEDIUM
- [LC25 · K 个一组翻转链表](docs/80-hot100-cpp/problems/0025-reverse-nodes-in-k-group.md) · HARD
- [LC138 · 随机链表的复制](docs/80-hot100-cpp/problems/0138-copy-list-with-random-pointer.md) · MEDIUM
- [LC148 · 排序链表](docs/80-hot100-cpp/problems/0148-sort-list.md) · MEDIUM
- [LC23 · 合并 K 个升序链表](docs/80-hot100-cpp/problems/0023-merge-k-sorted-lists.md) · HARD
- [LC146 · LRU 缓存](docs/80-hot100-cpp/problems/0146-lru-cache.md) · MEDIUM

### 二叉树 · 15题

- [LC94 · 二叉树的中序遍历](docs/80-hot100-cpp/problems/0094-binary-tree-inorder-traversal.md) · EASY
- [LC104 · 二叉树的最大深度](docs/80-hot100-cpp/problems/0104-maximum-depth-of-binary-tree.md) · EASY
- [LC226 · 翻转二叉树](docs/80-hot100-cpp/problems/0226-invert-binary-tree.md) · EASY
- [LC101 · 对称二叉树](docs/80-hot100-cpp/problems/0101-symmetric-tree.md) · EASY
- [LC543 · 二叉树的直径](docs/80-hot100-cpp/problems/0543-diameter-of-binary-tree.md) · EASY
- [LC102 · 二叉树的层序遍历](docs/80-hot100-cpp/problems/0102-binary-tree-level-order-traversal.md) · MEDIUM
- [LC108 · 将有序数组转换为二叉搜索树](docs/80-hot100-cpp/problems/0108-convert-sorted-array-to-binary-search-tree.md) · EASY
- [LC98 · 验证二叉搜索树](docs/80-hot100-cpp/problems/0098-validate-binary-search-tree.md) · MEDIUM
- [LC230 · 二叉搜索树中第 K 小的元素](docs/80-hot100-cpp/problems/0230-kth-smallest-element-in-a-bst.md) · MEDIUM
- [LC199 · 二叉树的右视图](docs/80-hot100-cpp/problems/0199-binary-tree-right-side-view.md) · MEDIUM
- [LC114 · 二叉树展开为链表](docs/80-hot100-cpp/problems/0114-flatten-binary-tree-to-linked-list.md) · MEDIUM
- [LC105 · 从前序与中序遍历序列构造二叉树](docs/80-hot100-cpp/problems/0105-construct-binary-tree-from-preorder-and-inorder-traversal.md) · MEDIUM
- [LC437 · 路径总和 III](docs/80-hot100-cpp/problems/0437-path-sum-iii.md) · MEDIUM
- [LC236 · 二叉树的最近公共祖先](docs/80-hot100-cpp/problems/0236-lowest-common-ancestor-of-a-binary-tree.md) · MEDIUM
- [LC124 · 二叉树中的最大路径和](docs/80-hot100-cpp/problems/0124-binary-tree-maximum-path-sum.md) · HARD

### 图论 · 4题

- [LC200 · 岛屿数量](docs/80-hot100-cpp/problems/0200-number-of-islands.md) · MEDIUM
- [LC994 · 腐烂的橘子](docs/80-hot100-cpp/problems/0994-rotting-oranges.md) · MEDIUM
- [LC207 · 课程表](docs/80-hot100-cpp/problems/0207-course-schedule.md) · MEDIUM
- [LC208 · 实现 Trie (前缀树)](docs/80-hot100-cpp/problems/0208-implement-trie-prefix-tree.md) · MEDIUM

### 回溯 · 8题

- [LC46 · 全排列](docs/80-hot100-cpp/problems/0046-permutations.md) · MEDIUM
- [LC78 · 子集](docs/80-hot100-cpp/problems/0078-subsets.md) · MEDIUM
- [LC17 · 电话号码的字母组合](docs/80-hot100-cpp/problems/0017-letter-combinations-of-a-phone-number.md) · MEDIUM
- [LC39 · 组合总和](docs/80-hot100-cpp/problems/0039-combination-sum.md) · MEDIUM
- [LC22 · 括号生成](docs/80-hot100-cpp/problems/0022-generate-parentheses.md) · MEDIUM
- [LC79 · 单词搜索](docs/80-hot100-cpp/problems/0079-word-search.md) · MEDIUM
- [LC131 · 分割回文串](docs/80-hot100-cpp/problems/0131-palindrome-partitioning.md) · MEDIUM
- [LC51 · N 皇后](docs/80-hot100-cpp/problems/0051-n-queens.md) · HARD

### 二分查找 · 6题

- [LC35 · 搜索插入位置](docs/80-hot100-cpp/problems/0035-search-insert-position.md) · EASY
- [LC74 · 搜索二维矩阵](docs/80-hot100-cpp/problems/0074-search-a-2d-matrix.md) · MEDIUM
- [LC34 · 在排序数组中查找元素的第一个和最后一个位置](docs/80-hot100-cpp/problems/0034-find-first-and-last-position-of-element-in-sorted-array.md) · MEDIUM
- [LC33 · 搜索旋转排序数组](docs/80-hot100-cpp/problems/0033-search-in-rotated-sorted-array.md) · MEDIUM
- [LC153 · 寻找旋转排序数组中的最小值](docs/80-hot100-cpp/problems/0153-find-minimum-in-rotated-sorted-array.md) · MEDIUM
- [LC4 · 寻找两个正序数组的中位数](docs/80-hot100-cpp/problems/0004-median-of-two-sorted-arrays.md) · HARD

### 栈 · 5题

- [LC20 · 有效的括号](docs/80-hot100-cpp/problems/0020-valid-parentheses.md) · EASY
- [LC155 · 最小栈](docs/80-hot100-cpp/problems/0155-min-stack.md) · MEDIUM
- [LC394 · 字符串解码](docs/80-hot100-cpp/problems/0394-decode-string.md) · MEDIUM
- [LC739 · 每日温度](docs/80-hot100-cpp/problems/0739-daily-temperatures.md) · MEDIUM
- [LC84 · 柱状图中最大的矩形](docs/80-hot100-cpp/problems/0084-largest-rectangle-in-histogram.md) · HARD

### 堆 · 3题

- [LC215 · 数组中的第K个最大元素](docs/80-hot100-cpp/problems/0215-kth-largest-element-in-an-array.md) · MEDIUM
- [LC347 · 前 K 个高频元素](docs/80-hot100-cpp/problems/0347-top-k-frequent-elements.md) · MEDIUM
- [LC295 · 数据流的中位数](docs/80-hot100-cpp/problems/0295-find-median-from-data-stream.md) · HARD

### 贪心算法 · 4题

- [LC121 · 买卖股票的最佳时机](docs/80-hot100-cpp/problems/0121-best-time-to-buy-and-sell-stock.md) · EASY
- [LC55 · 跳跃游戏](docs/80-hot100-cpp/problems/0055-jump-game.md) · MEDIUM
- [LC45 · 跳跃游戏 II](docs/80-hot100-cpp/problems/0045-jump-game-ii.md) · MEDIUM
- [LC763 · 划分字母区间](docs/80-hot100-cpp/problems/0763-partition-labels.md) · MEDIUM

### 动态规划 · 10题

- [LC70 · 爬楼梯](docs/80-hot100-cpp/problems/0070-climbing-stairs.md) · EASY
- [LC118 · 杨辉三角](docs/80-hot100-cpp/problems/0118-pascals-triangle.md) · EASY
- [LC198 · 打家劫舍](docs/80-hot100-cpp/problems/0198-house-robber.md) · MEDIUM
- [LC279 · 完全平方数](docs/80-hot100-cpp/problems/0279-perfect-squares.md) · MEDIUM
- [LC322 · 零钱兑换](docs/80-hot100-cpp/problems/0322-coin-change.md) · MEDIUM
- [LC139 · 单词拆分](docs/80-hot100-cpp/problems/0139-word-break.md) · MEDIUM
- [LC300 · 最长递增子序列](docs/80-hot100-cpp/problems/0300-longest-increasing-subsequence.md) · MEDIUM
- [LC152 · 乘积最大子数组](docs/80-hot100-cpp/problems/0152-maximum-product-subarray.md) · MEDIUM
- [LC416 · 分割等和子集](docs/80-hot100-cpp/problems/0416-partition-equal-subset-sum.md) · MEDIUM
- [LC32 · 最长有效括号](docs/80-hot100-cpp/problems/0032-longest-valid-parentheses.md) · HARD

### 多维动态规划 · 5题

- [LC62 · 不同路径](docs/80-hot100-cpp/problems/0062-unique-paths.md) · MEDIUM
- [LC64 · 最小路径和](docs/80-hot100-cpp/problems/0064-minimum-path-sum.md) · MEDIUM
- [LC5 · 最长回文子串](docs/80-hot100-cpp/problems/0005-longest-palindromic-substring.md) · MEDIUM
- [LC1143 · 最长公共子序列](docs/80-hot100-cpp/problems/1143-longest-common-subsequence.md) · MEDIUM
- [LC72 · 编辑距离](docs/80-hot100-cpp/problems/0072-edit-distance.md) · MEDIUM

### 技巧 · 5题

- [LC136 · 只出现一次的数字](docs/80-hot100-cpp/problems/0136-single-number.md) · EASY
- [LC169 · 多数元素](docs/80-hot100-cpp/problems/0169-majority-element.md) · EASY
- [LC75 · 颜色分类](docs/80-hot100-cpp/problems/0075-sort-colors.md) · MEDIUM
- [LC31 · 下一个排列](docs/80-hot100-cpp/problems/0031-next-permutation.md) · MEDIUM
- [LC287 · 寻找重复数](docs/80-hot100-cpp/problems/0287-find-the-duplicate-number.md) · MEDIUM

### 额外公开练习：不计入官方100

- [LC695 · 岛屿的最大面积](docs/80-hot100-cpp/supplementary/0695-max-area-of-island.md)
