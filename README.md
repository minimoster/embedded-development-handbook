# 嵌入式开发 · 学习与面试手册

从可解释的机制出发，把语言、硬件、系统和工程验证连成一条主线。中文教学讲义、C/C++主机练习、逐题追问与验收、可追溯来源，以及适合手机/电脑的静态阅读器。

## 开始学习

1. [学习路线](docs/00-guide/learning-path.md)：共享基础→MCU/RTOS或Linux主方向→项目答辩，按阶段验收推进。
2. [题目索引](QUESTION_INDEX.md)：按主题查找，不背孤立名词。
3. [主机实验](examples/README.md)：边界、分片、回绕、资源和线程关闭；先测硬件无关逻辑。
4. [C++ Hot100](docs/80-hot100-cpp/README.md)：官方100题与LC695额外公开练习，独立题解/源码/测试。
5. [公开面经与原创故障场景](docs/120-interviews/README.md)：17份新增完整自报页面、7卡追问，日期/岗位边界单列。
6. [FreeRTOS 源码导读](docs/50-rtos/source-reading.md)：7篇原创扩展讲义，沿链表、任务、队列、事件组与共享daemon读固定版本；历史co-routines单独选读。
7. [可选深化讲义](docs/130-reference/optional-depth.md)：16篇接口与验证合同选读，接在已有主线之后，单独计数。
8. [版本与来源](VERSIONS.md)、[资料阅读状态](sources/README.md)、[覆盖边界](docs/130-reference/coverage-boundaries.md)。

## 阅读方式

- 下载后打开site/index.html：离线浏览、全文搜索、目录、折叠解答、字号/行距、明暗主题、收藏/已读与继续阅读。
- [阅读器说明](site/README.md)：本地预览、确定性构建与验证。阅读记录只在浏览器本地，无账号/追踪/上传。
- [在线阅读器](https://minimoster.github.io/embedded-development-handbook/)：手机/电脑浏览与搜索。GitHub内也可直接读docs/；源码和测试可单独检查。

## 课程结构

- C/C++与内存：指针/长度、生命周期、const/volatile/atomic、UB、布局/序列化、分配、RAII、移动与多态。
- STL：容器/迭代器失效、字符串视图、哈希/排序/复杂度与实时上界。
- 架构/构建：编译链接、ELF、VMA/LMA、启动、NVIC、MSP/PSP、ABI、交叉编译。
- STM32/RTOS：GPIO、timer、ADC、UART、DMA/cache、消抖、看门狗、Flash；任务、同步、ISR接口、周期与资源预算。
- 协议/网络：I²C/SPI/CAN/RS485概念，应用帧，TCP/UDP、重试幂等、IP/ARP/DNS/TLS与安全边界。
- Embedded Linux：进程/线程、短读写、epoll、条件变量、mmap/IPC/信号、设备接口、驱动模型、DT/MMIO/DMA、内核并发。
- 启动/工程：U-Boot可选阶段、kernel/DTB/rootfs/init、Buildroot、升级/回滚；调试、测试、故障注入与项目表达。

每个核心题：直接回答→原理拆解→例子/推演→误区/失败边界→追问→动手验收→官方来源。代码片段与已经运行的完整练习分开记录；题数在manifest中生成，避免手填不一致。

## 实际覆盖与权利边界

课程提供主题讲义、可运行主机练习、官方题目学习入口和公开技术文档引用。外部题目与官方文档的权利归各自权利人，公开链接不表示转载许可。[权利说明](LICENSE.md)。

主机测试通过不代表STM32、FreeRTOS端口、Linux驱动或真实OTA已经运行。硬件/内核测试未做，LeakSanitizer本容器受限，[验证说明](coverage/README.md)记录具体范围。面经便利样本不是统一考频，学习优先级也不是招聘概率。

## 构建与校验

```bash
python3 scripts/update_index.py
python3 scripts/validate.py --strict
python3 scripts/validate_public_metadata.py
python3 tests/run_host_tests.py
python3 tests/run_protocol_depth_tests.py
python3 tests/run_interview_depth_tests.py
node site/build.mjs
node site/build.mjs --check
node site/test.mjs
```

无需安装npm依赖。外设/内核内容使用前固定实际芯片、工具链、Kernel/RTOS版本与配置；纠错请给题号、平台和可复现证据。
