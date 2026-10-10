# 版本与适用范围

核查日期：2026-10-10。

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
