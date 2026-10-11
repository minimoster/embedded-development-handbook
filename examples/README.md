# 主机实验：先把硬件无关逻辑测稳

## 可运行代码

- [C核心逻辑](c/embedded_core.c)与[头文件](c/embedded_core.h)：位掩码、溢出前检查、大端解码、tick回绕、顺序ring、消抖与流式解析。
- [C边界测试](../tests/host_core_test.c)：32个合法bit、非法输入、回绕；50000步ring模型对照；11字节流的全部1024分片组合；坏长度/校验、截断与最大帧；消抖阈值与回绕。
- [C++资源/有界队列](cpp/resources.hpp)与[测试](../tests/host_cpp_test.cpp)：move-only资源责任；10000项FIFO；空/满队列停止、重复停止与join；[受控等待与分配失败回归](../tests/host_queue_regression_test.cpp)。
- [精确数值解析](c/numeric_parse.c)与[接口](c/numeric_parse.h)、[边界测试](../tests/host_numeric_test.c)：INT32_MIN/MAX、非法span、20001往返与50000小域int64对照。
- [Linux进程/fd实验](../tests/host_linux_lifecycle_test.c)：fork/exec/wait回收、dup共享offset与unlink后打开fd存活，只操作受控子进程与临时文件。
- [C++语言实验](../tests/host_cpp_language_test.cpp)：成员初始化、虚派发/默认参数、安全lambda捕获与容器适配器。
- [测试入口](../tests/run_host_tests.py)：编译器警告按错误处理，有限执行超时。

```bash
python3 tests/run_host_tests.py
python3 tests/run_host_tests.py --sanitize
```

本次容器的LeakSanitizer因ptrace限制不能工作。要在相同受限环境单独运行ASan/UBSan，可显式加：

```bash
python3 tests/run_host_tests.py --sanitize --disable-leak-detection
```

这会清楚输出“LeakSanitizer disabled”，不是泄漏检查通过。在正常开发机优先不加此参数。代码基线是C11/C++17；主机编译器和结果见验证说明。

## 实验1：长度与字节序

先让read_be16拒绝不足2字节，再解码12 34。添加空指针、短输入和极值。把固定测试换成所有65536个uint16_t编码再解码，验证往返。不要强转未对齐byte buffer。

## 实验2：ring 的容量与回绕

ring留一槽，所以数组8项可存7项；只有顺序语义，没有ISR/线程/DMA并发保证。尝试把所有权/原子顺序设计写到另一个实现，不能仅添加volatile就改标题为lock-free。

## 实验3：流式解析

玩具格式是length(0..16)+payload+XOR(length,payload)。XOR只为教学方便，既非CRC也非认证。长度被拒绝后只从下字节重新尝试，未提供任意噪声下可靠重同步保证；正式协议还需同步头、转义、超时、版本与安全设计。

parser必须零初始化，callback不重入/改写该parser；回调payload只在本次回调读取期间有效；若要异步保留，必须复制到自己拥有的buffer。分片独立性测试证明有限合法stream切分下结果相同，不证明所有输入或网络安全。

## 实验4：非阻塞消抖

用虚拟tick，先输入抖动再稳定，确认仅产生一次状态变化。阈值20、恰好19/20、初始状态、回绕都要测。这里没有长按实现，读者可在已确认按下事件上增加独立状态。

## 实验5：RAII 与有界队列

Resource用计数器模拟close，计数器必须比资源活得长，且测试只在一个上下文操作资源。它不是已经连接真实fd的封装。Queue使用标准mutex/condition_variable，在host线程验证FIFO与关闭；不适用于ISR，不保证无堆分配或硬实时。队列要求元素移动构造/赋值不抛异常，并通过static_assert限制。deque单端插入分配失败时队列不变，catch先把仍可用的空位通知另一生产者，再将异常交给调用方；否则被唤醒者失败后，另一等待者可能持续睡眠。默认allocator与wait observer保持普通使用，测试替换allocator注入一次真实分配失败；observer在持有queue mutex时报告即将等待，测试随后取得同一mutex的pop/close不能抢在atomic unlock-and-wait之前执行，不依赖sleep证明线程已等待。回归重复10次，并在超时失败时先close/join再断言。线程调度、公平性、持续资源耗尽与系统同步错误不在进展保证内。

`push(T value)`在调用入口就接收值；move-only调用者用std::move传入后，即使返回false或插入抛异常，也不能假设原对象仍拥有资源。失败时参数由其析构释放，队列只拥有成功插入的元素。close停止新写入，pop排空已有项；所有使用线程必须先结束/join，再销毁queue。线程创建/加入及异常处理仍由调用方负责。

代码片段、机制推演和可运行文件分开记录。未运行STM32、FreeRTOS端口、Linux内核模块、目标板网络、功耗、Flash耐久或真实OTA。

## 实验6：精确十进制解析

parse_i32_span接受明确长度的ASCII十进制，编译期要求8位字节和ASCII兼容执行字符集，可选符号，至少一个数字，不接受空白/尾字符/嵌入零，失败不修改输出。先比较剩余容量再乘10，独立处理INT32_MIN，避免先溢出再检查。它比strtol合同更严格，不伪称两者完全等价；源字符串无需NUL结尾。

## 实验7：进程与fd生命周期

Linux fixture只fork自己的孩子，用waitpid回收并检查退出状态，exec同一测试二进制进入有界child分支。另用临时文件演示dup后的共享offset，以及unlink删除名字后已有fd仍能读取。依赖Linux的/proc/self/exe；不能据此声称BSP/内核模块/目标板已测。

## 实验8：C++对象和异步捕获

初始化顺序、默认实参和虚派发分别验证；捕获shared_ptr使被引用对象在回调期间活着，move-only捕获转移独占拥有者。裸引用和this悬空的错误版本不执行。通过有限测试不证明所有异步注销/并发情况，真实回调系统还需取消和在飞行任务同步。

数字解析另外运行[差分测试](../tests/host_numeric_differential_test.c)：216,403个有界span与strtoimax范围oracle比较，含边界附近、长零、任意字节与精确非NUL结尾输入；不是全输入穷举证明。

## 实验9：真正的C/C++ ABI边界

[共享头文件](cpp/device_boundary.h)、[C++实现](cpp/device_boundary.cpp)与[C调用方](../tests/host_c_abi_test.c)由脚本分别按C11/C++17编译、再链接。假计数资源通过opaque handle配对创建/销毁；测试空参数、耗尽后不改输出、空句柄销毁与非零限值。实现-fno-exceptions并使用nothrow分配；不模拟板上设备，不宣称分配失败注入或任意非法句柄可恢复。

## 协议深化：有限主机模型与 mock 接口

- [I²C模型](python/i2c_timing_recovery.py)与[20项测试](../tests/host_i2c_timing_recovery_test.py)：时序/上拉区间、逐边沿组合读以及2532条有限bus-clear故障轨迹。
- [SPI接口](c/spidev_transaction.h)、[C实现](c/spidev_transaction.c)与[mock测试](../tests/host_spidev_transaction_test.c)：四模式理想边沿和单消息命令/数据，含配置失败、异常返回与长度；不访问真实设备。
- [经典Reno/RTO模型](python/tcp_reno_rto.py)与[19组测试](../tests/host_tcp_reno_rto_test.py)：累计ACK、窗口、快恢复、超时/退避和保守Karn边界；无真实socket或网络。
- [统一入口](../tests/run_protocol_depth_tests.py)：Python模型及严格C编译；可为C mock追加ASan/UBSan。

```bash
python3 tests/run_protocol_depth_tests.py
python3 tests/run_protocol_depth_tests.py --sanitize
# 仅在ptrace限制环境中显式关闭泄漏检测；不算LSan通过：
python3 tests/run_protocol_depth_tests.py --sanitize --disable-leak-detection
```

这是原创有限教学模型与同步mock，不是总线/网络模拟器、内核符合性认证或板测。具体假设、来源版本和手推轨迹在三章分别说明；不推断芯片寄存器、设备恢复政策、板级上拉电阻或Linux默认TCP算法。
