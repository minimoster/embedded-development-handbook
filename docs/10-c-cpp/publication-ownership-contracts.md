# 发布完成之后，谁还能碰这块内存？C++17 双向所有权交接实验

学习优先级：P1 深化。适合已经能回答 C04、C08、C09、RT09，但还无法把“发布完成”“消费完成”“允许复用”连成一份可执行合同的读者。

本章是原创主机教学实现，不替换原来的顺序 ring 或 mutex/condition_variable 队列。只讨论一个生产者和一个消费者之间的单槽交接。代码限定 C++17，不使用 C++20 的 atomic::wait、notify 或 jthread。章节中的逐步证明是对本实现的推导；有限运行结果与标准条款是不同证据。

## 1. 面试先给出分层答案

### 30 秒答案：必须提到“归还”，否则只答了一半

> 我先限定单生产者、单消费者、两端不重入、槽位存活到所有使用线程结束。生产者拿到空槽后写普通 payload，再 release 发布；消费者 acquire 取得本轮消息，读完或移动到自己拥有的输出之后，再 release 归还槽位；生产者下一轮 acquire 接收归还才允许覆盖。两条边分别保证消息可读和旧消息不会被提前覆盖。借用指针不能在归还后继续使用，移动资源也必须通过这份交接合同。

评分：只说“atomic 防竞争”是 1 分；讲清生产者到消费者是 2 分；补上消费者到生产者、借用范围和前提是合格；能说出第二次覆盖前的读写冲突由哪条边排序，才是扎实答案。

### 2 分钟答案：把失败语义一起说清

> 我用一个常驻 payload 和一个 atomic<bool> ready。ready 为 false 是生产者写入许可，为 true 是消费者读取许可。try_publish(T& source) 在满时直接返回 false，不移动 source；空时才移动赋值 payload，随后 release-store true。try_consume(T& output) 在空时不改 output；取得 true 后先完成移动赋值，再 release-store false。输出的旧资源可能在移动赋值过程中释放，所以 T 的移动赋值要不抛异常，且资源释放成本不是原子标志能控制的。
>
> 对固定数组消息，移动通常仍复制数组内容；对 unique_ptr，移动转交管理对象的释放责任，源变空。消费者移动出的独立资源可以继续使用，槽位本身的引用则不能保留。我另外提供 noexcept 的访问回调，回调结束前不归还槽位。测试会在回调内设置可观测入口信号，让生产者尝试写入并检查拒绝；不靠 sleep 猜测消费者是否已经开始读。
>
> 证明还要解释 bool 重复取值：生产者自己上次写 true 在本线程下一次 load 之前；原子对象的一致性约束不能让它倒退读到更早的 false。当它取得新的 false，取得的是消费方后续归还。消费方同理。主机 stress 成功只检查被执行的轨迹，不能替代这个论证，也不能推出目标 MCU 的 atomic 是无锁或可在 ISR/DMA 使用。

评分：补充失败保持源/输出、不抛异常边界、反向发布、重复值的新旧轮次和验证限度，才算完整回答。若只说“加 release/acquire 就行”，继续追问第 5 节。

### 深答：现场画四个动作和两个对象

不要先罗列所有 memory_order。先画出两个对象：原子许可 ready、普通 payload。再标出本轮 W、发布 P、取得 A、读/移出 R，以及归还 F、下一轮取得 E。回答“哪个对象原子、哪些普通访问之间必须有 happens-before、资源去哪了”。下文提供可与源文件逐行对应的展开。

## 2. 最小需求与明确不做的事情

场景：采集线程产生一条最多 32 字节的 Packet，处理线程接收；处理没结束之前不允许采集线程复用同一块存储。采用容量恰好为 1 的交接槽，避免先被环索引、留空格和计数器回绕分散注意。

合同逐项写清：

1. 构造结束后再启动线程；槽位比所有调用者活得久，销毁前所有调用者都已退出并成功 join。
2. 全部生产调用只有一个串行调用者；全部消费调用只有一个串行调用者。try_consume 与 try_visit 属于同一消费端，不能由两个人同时调用，也不能在回调中重入。
3. source 和 output 分别由调用者独占，不与槽位内存别名，不与另一端正在操作的外部对象别名。对象指向的可变状态也遵守所有权合同；“外层 unique_ptr 独占”不能修复泄漏出去的裸指针访问。
4. payload 的 T 对象始终存在。交接改变访问许可和 T 所定义的资源责任，没有每轮 placement new、析构后复活或对象地址搬迁。
5. T 必须可不抛异常地默认构造、移动赋值和析构。编译期检查这些条件，依然无法检查“移动函数里面偷偷访问别的共享对象”等语义错误。
6. 每次 try 调用仅尝试一次，不内置等待。没有 close、reset、并行销毁或取消在途操作的接口。
7. 主机 C++ 线程模型下的正确性是本章范围。目标 lock-freedom、ISR 约束、DMA/cache 协议、调度公平性和时间上界另外审查。

ready 不是“缓存里有最新值”的提示，也不是对外查询队列长度的快照；它是决定哪一端能碰 payload 的许可。失败必须停在原子 load 后，不得先读 payload 再判断许可。

## 3. 交付代码与第一次运行

- [完整交接槽](../../examples/cpp/spsc_publication_slot.hpp)：唯一实现，含类型约束和借用合同。
- [固定消息与独立检查函数](../../examples/cpp/publication_packet.hpp)：sequence、length、32 字节数组、教学 checksum。
- [有界演示](../../examples/cpp/publication_demo.cpp)：32 条消息，5 秒观测截止，线程结束后检查结果。
- [定向测试](../../tests/host_publication_ownership_test.cpp)：7 组运行合同。
- [独立入口](../../tests/run_publication_ownership_tests.py)：只编译运行本章，不重跑旧套件。

在仓库根目录执行：

```bash
python3 tests/run_publication_ownership_tests.py --report validation/publication-normal.json
python3 tests/run_publication_ownership_tests.py --sanitize --report validation/publication-sanitizer-default.json
# 仅当受限宿主的默认 LSan 报告 ptrace 不支持时，显式使用：
python3 tests/run_publication_ownership_tests.py --sanitize --disable-leak-detection --report validation/publication-sanitizer-no-leaks.json
```

演示与测试最终使用的严格编译参数：

```bash
g++ -std=c++17 -pthread -Wall -Wextra -Wpedantic -Werror \
    -Wconversion -Wsign-conversion -Wshadow -O1 -g \
    examples/cpp/publication_demo.cpp -o /tmp/publication-demo
/tmp/publication-demo
```

此手动命令是复现方式；本次实际执行的是上方 Python 入口。入口可生成各阶段命令和结果的 JSON；本次实测范围见下方验收说明。ASan/UBSan 再加 -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer。不是向全项目添加 sanitizer 配置。

消息校验只用于发现这次实验里的混合字段、错序和丢失。checksum 不是 CRC、密码学认证，也不是通信协议。length 不参与越界索引；槽位只运输 T，不验证业务长度。测试专门证明 Packet 检查器拒绝 length=33，没有声称槽位拒绝这个对象。

## 4. 两段核心代码：成功线才允许触碰 payload

下面与完整头文件的核心操作一致；类型约束、删除复制/移动和注释请看源文件。

```cpp
bool try_publish(T& source) noexcept {
    if (ready_.load(std::memory_order_acquire)) {
        return false;
    }
    payload_ = std::move(source);
    ready_.store(true, std::memory_order_release);
    return true;
}

bool try_consume(T& output) noexcept {
    if (!ready_.load(std::memory_order_acquire)) {
        return false;
    }
    output = std::move(payload_);
    ready_.store(false, std::memory_order_release);
    return true;
}
```

这不是把普通 T 整体做成 atomic。原子只用于许可；payload 仍是普通对象，其安全来自禁止不排序的冲突访问。这里没有“自己的索引读可以 relaxed”的位置，因为双方读取的同一个 ready 还承载对方完成动作的发布。RT09 中的 head/tail 是另一份双索引合同，不应机械搬来。

try_publish 的参数故意写成 T&：检查失败之前没有按值构造参数。对 unique_ptr，成功后源为空，失败后仍然拥有原对象；对一般 T，成功后只保证源处于其移动合同允许的状态，不保证数值清零。测试 Packet 是平凡成员组成的值类型，移动后数组内容仍在，不应把这个结果套给所有 T。

try_consume 在移动赋值结束之后才写 false。消费者之后处理独立 output 可以很慢，槽位仍可供下一轮使用，前提是移出后的对象不再借用槽位存储。含 raw pointer、string_view 或自引用字段的自定义类型要重新检查这一前提。

## 5. 逐动作证明：发布与复用是两条不同的边

先固定术语与依据：release/acquire 的同步必须匹配所读出的发布；happens-before 可结合线程内顺序传递；普通冲突访问缺少排序会形成数据竞争。原子对象还有修改顺序与一致性约束。依据是固定 [N4659 §4.7.1 [intro.races] ¶2、4、6、9–10、14–17、20](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/n4659.pdf#page=29) 与 [§32.4 [atomics.order] ¶1–2](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/n4659.pdf#page=1360)。这份公开工作草案日期为 2017-03-21，不是正式 ISO 出版文本。

以下是针对本实现的原创推导。记 SB 为同一线程内 sequenced-before，SW 为匹配发布的 synchronizes-with，HB 为 happens-before。

### 5.1 生产者到消费者：为什么读到本轮完整数据

第 k 轮：

- Wk：生产者对 payload_ 完成移动赋值，含各字段写入。
- Pk：生产者 release-store true。
- Ak：消费者成功 acquire-load，从 Pk 取得 true。
- Rk：消费者把 payload_ 移动赋值给 output，或在回调中读取。

链条为 Wk SB Pk SW Ak SB Rk，因此 Wk HB Rk。普通 payload 的本轮写入和本轮读取有排序关系。成立的关键是 Ak 取得对应的发布许可；不是任何 acquire 都会自动同步任意 release，更不是“前面用了一个原子，所以后面全部安全”。

### 5.2 消费者到生产者：为什么下一轮不会覆盖正在读的字节

再标出：

- Fk：Rk 完成之后，消费者 release-store false。
- Ek+1：下一轮生产者成功 acquire-load，从 Fk 取得 false。
- Wk+1：生产者开始覆盖 payload_。

链条为 Rk SB Fk SW Ek+1 SB Wk+1，因此 Rk HB Wk+1。若 Rk 的移动还会修改槽位的源对象，这条边也排序这些修改。要防止覆盖，本轮读取结束的发布与下一轮复用的取得同样重要。

把两段接起来：W0 → R0 → W1 → R1 → W2。箭头表示 HB 关系，不表示操作必须逐个机器指令轮流执行。处理 output 的后续工作不在槽位内，因此可以与下一轮写入重叠。

### 5.3 bool 只有两种值，为什么不会读到旧一轮 false/true

只画值的时间线不足以证明复用，必须把每次 store 视为不同事件。

初态 I 写入 false，随后合法修改序列是：I(false)、P0(true)、F0(false)、P1(true)、F1(false)……本算法没有第三个状态写者，没有 reset，也没有 RMW/release sequence 技巧。

- 生产者自己完成 Pk 后，本线程后续 load 在 Pk 之后。它不能把 Pk 以前的 false 当成新许可。若取得 false，只能来自更后的归还。生产者没有获得下一次许可之前，自己也不会再发布下一轮 true。
- 消费者自己完成 Fk 后，本线程后续 load 在 Fk 之后。它不能再次把 Fk 以前的 true 当成新消息。若取得 true，取得的是后续生产发布。消费者在取得新消息之前不会再次写 false。
- 第一次生产取得 I(false) 不需要消费端同步，因为 payload 尚未被消费方访问；初始构造与启动线程另有生命周期保障。第一次消费则仍必须取得 P0(true)。

可以再把归纳写得更精确：同端上一次 store 到下一次成功 load 的 write-read 约束，给“load 所读修改”一个下界；本次 load 到紧接的本次 store 的 read-write 约束，给它一个上界。如果修改顺序中出现连续两个生产 store，它们之间必须存在一个让第二次取得成功的 false 修改，与“连续”矛盾；连续两个消费 store 同理。第一次消费 store 前还必须已经有一个 true 发布。因此本实现的 store 交替不是凭时间线假定出来，而是由许可条件、两端串行和原子一致性约束共同推出。标准的 HB 关系也不能成环，不能借一个需要本轮发布之后才产生的未来归还来循环授予本轮许可。不能省略这一部分，把“bool 值相等”直接等同于“属于同一轮”。多端、重入、额外 reset 或让第三者归还都会破坏该归纳，届时需要重新设计状态机，不能继续引用本证明。

读取可以暂时看到拒绝条件，于是 try 返回 false；这是可重试的保守失败，不会读写 payload。原子一致性和本算法的安全论证没有给出每次 load 多快看到对方、哪个线程多久被调度的时间承诺。

## 6. 把第二条边手推到字节复用

设初始 Packet A：sequence=701，length=32；下一条 B：sequence=702，length=1。

| 步骤 | 当前许可 | 生产者动作 | 消费者动作 | 可以复用吗 |
|---|---|---|---|---|
| 1 | false | 获得许可，写 A | 尚未取得消息 | 生产者有写许可 |
| 2 | true | 完成 release 发布 A | acquire 取得 A | 不能写 B |
| 3 | true | 尝试 B，返回 false，B 保持原样 | 回调内仍借用 A | 不能写 B |
| 4 | true | 仍只能拒绝 | 在回调尾部再次核对 A 全部字段 | 仍不能写 B |
| 5 | false | 尚未重新取得归还 | 回调返回之后 release 归还 | 可申请下一次写许可 |
| 6 | true | acquire 归还，写 B，再 release | 下一轮 acquire 后才读 B | B 已发布 |

受控测试的 entered 原子只告诉主测试“回调已经进入”；leave 原子只让回调退出。它们不是生产数据的另一条发布通路：A 必须由槽位的 ready 获取，B 在 A 的回调内必须被槽位拒绝。测试在借用末尾读 A，避免只在开头读取就错过“读到一半被覆盖”。所有测试断言都安排在释放 leave 和 join 之后，断言失败也不会把一个 joinable 线程丢进析构。

这个受控轨迹检验“读完前拒绝覆盖”的实现行为；它仍不是全内存模型枚举。时间截止是测试避免无限挂起的观察边界，不是算法的 WCET。

## 7. 三种“拥有”不要混在一起

### 7.1 内存对象生命周期

生命周期核验点为 [N4659 §6.8 [basic.life] ¶1](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/n4659.pdf#page=88)；下面只分析这个常驻成员的使用合同。

slot 中的 payload_ 是常驻 T，从 slot 构造到析构一直活着。把 ready 写 false 不会结束 T 的生命周期。借用后保存一个指向 payload_ 的指针，地址可能仍有效，内容却已经属于下一轮；这首先是跨轮访问许可错误，若与覆盖并发还会形成冲突。不能只问“有没有 free”。

### 7.2 消息访问许可

成功 publish 将槽位访问许可交给消费者；成功 consume/visit 结束时将访问许可交回生产者。这里的“所有权”是一份并发访问合同，不表示底层内存每轮 free/malloc，也不表示用户可直接访问私有 payload_。

### 7.3 T 管理的资源责任

`SpscPublicationSlot<std::unique_ptr<Counted>>` 的测试独立检查：

- 第一次发布成功，调用者的 first 为空，槽位拥有原对象。
- 满槽拒绝 second，second 的地址和拥有关系保持，销毁计数仍为 0。
- 消费者移出 first，取得同一个地址，槽位不再负责它；下一轮复用槽位不影响移出的 first。
- 用第二次 consume 覆盖消费者 output，旧资源由 output 的移动赋值释放一次。
- 留一条 pending 不消费，所有调用已结束之后析构 slot，释放 pending 一次；最后消费者 reset output，总计三次。

这是本测试中三件对象的责任检查，不是对任意 deleter 的证明。自定义 deleter、资源关闭线程要求、阻塞时间和可达外部状态仍属于类型合同。C++17 unique_ptr 的移出责任核对见 [N4659 §23.11.1.2.3 [unique.ptr.single.asgn] ¶1–3 与 §23.11.1.2.2 [unique.ptr.single.dtor] ¶1–2](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/n4659.pdf#page=628)。

try_visit 是“借用然后丢弃这条消息”，但 callback 返回不会立刻销毁 payload_。若 T=unique_ptr，旧资源可能到下一次生产移动赋值或者 slot 析构时才释放。因此这个接口没有“归还许可就立即释放资源”的承诺；要求严格释放时点或释放线程的系统，应另外设计明确的 dispose 或移出路径。

回调拿到 const T& 并不让全部可达对象深层只读。比如 const unique_ptr 仍可指向可变对象；合同仍禁止把其可变状态给外部线程并发修改。const 是接口约束的一部分，不能代替发布和生命周期证明。

## 8. 常见错误：指出缺失关系，不运行 UB 版本

| 变动 | 首先破坏什么 | 应怎样回答追问 |
|---|---|---|
| ready 换成普通 bool 或 volatile bool | 许可本身的并发访问与发布证明 | 找出冲突，说明为什么缺少同步；不把打印正常当依据 |
| 正向发布/取得换 relaxed | Wk 到 Rk 的排序依据 | 原子许可仍原子，但普通 payload 没有这条发布保护 |
| 反向归还/取得换 relaxed | Rk 到 Wk+1 的复用排序依据 | 不能凭当前 CPU 读写看起来按序而跳过 HB |
| 先 store(false)，再复制或移动 payload | 归还动作早于读取完成 | 即使用 seq_cst，源代码给出的许可时间仍然错 |
| 成功 visit 后保存借用引用，另起异步任务读 | 回调结束后的许可和对象内容归属 | 必须独立复制、移出拥有者，或延长明确借用协议 |
| 用 shared_ptr 代替 unique_ptr 就认为字段安全 | 可达对象的可变字段合同 | 延长生命周期与并发访问控制是两件事 |
| 两个生产者都 load(false) 后写 | SPSC 独占端点前提 | 两个人可同时进入普通 payload 写入；不能只说 atomic 已用 |
| 两个消费者分别 consume/visit | 单消费者前提 | 两个人可操作同一 payload、重复归还 |
| callback 重入消费操作 | 串行、不重入前提 | 本轮 ready 尚为 true，不防重复进入 |
| 热运行 reset ready=false | 许可轮次与生命周期 | 第三方状态写者能制造提前复用；只能先结束所有使用者再考虑重建 |
| 外部 source/output 指向槽位或彼此共享对象 | 别名/独占前提 | 原子保护不覆盖任意外部引用和指针 |
| 把 noexcept 当作“没有失败/不会阻塞” | 异常和实时性的混淆 | noexcept 只限制异常传播，不能约束阻塞、分配、外部锁或 terminate |

以上只做推理，不提供可执行的竞态变体。不会为了演示“可能撕裂”而运行一个已经违反 C++ 数据竞争规则的程序。

关于 volatile 的实现层边界，核对的是 [GCC 14.2 §6.47 Volatiles](https://gcc.gnu.org/onlinedocs/gcc-14.2.0/gcc/Volatiles.html)：GCC 不把 volatile 访问当作普通非 volatile 访问的排序屏障。本章没有 MMIO 访问；不要把 C++ 主机线程同步方案搬去寄存器访问。

## 9. 越问越深的六条面试链

### 链 A：读到新 flag，为什么还会担心旧数据？

1. ready 原子，是不是 Packet 自动原子？答：不是；成功读取前的同步把普通字段访问排序。
2. acquire 是不是强制从“最新缓存”取？答：用读取来源和同步关系解释，避免以缓存口号代替 C++ 合同。
3. 只要一侧 release，另一侧 relaxed 可不可以？答：本实现的直接发布取得缺少对应 acquire，不能继续用这份证明。
4. 错误版本在 x86 连跑成功呢？答：成功轨迹无法修复语言层无竞争证明，也不覆盖另一个编译器/目标。

### 链 B：为什么消费端也要发布？

1. 生产者传数据给消费者，消费者没有“新数据”，release false 有什么作用？答：发布“读/移动已经完成”。
2. 读完后处理 output 很久会卡槽位吗？答：真正独立的 output 已取得，槽位可复用；若 output 借用槽位就不独立。
3. 所有原子升级为 seq_cst，能先归还再读吗？答：不能，错误的程序顺序仍允许复用早于读取。
4. 换成三缓冲就不需要归还吗？答：仍须有每块 buffer 的占用、发布和回收责任，只是允许更多在途项。

### 链 C：move、引用与资源释放

1. try_publish(std::move(p)) 为什么本接口不能这样写？答：接口要求 T&，调用 p，让成功路径内部决定移动；失败前不夺走所有权。
2. 那为什么原 mutex 队列按值收参数？答：它采用另一份接收语义；不可把失败后保持源的结论套给按值接口。
3. Packet 的 move 是否零拷贝？答：数组字段仍需转移值；不应仅凭 std::move 这个拼写声称零拷贝。
4. unique_ptr 移动可以证明任意裸观察指针都安全吗？答：不可以；另有观察者必须遵循其对象生命周期和并发访问协议。

### 链 D：多端与重入

1. 为什么不用 CAS？答：合同排除了竞争领取同一个端点的两个人，另一端只能在当前许可交接后改变状态。
2. 两个生产者加一个 CAS(false,true) 可以直接修好吗？答：这把“占用写槽”与“数据已发布”合成同一 true，消费者可能在写完前看到 true；状态机需要区分保留和提交。
3. MPSC 用 fetch_add 得到索引是不是够了？答：还需证明容量、提交完成顺序、洞、失败恢复和槽位代次。
4. ISR 抢占任务但都调用生产接口算一个生产者吗？答：不是本章所说的串行、非重入端点；嵌套/抢占要独立证明。

### 链 E：停止与生命期

1. count 已发完，是不是析构 slot？答：等消费完成，结束并 join 全部使用者。
2. relaxed abort 是不是用来发布最后一条 Packet？答：不是，本实验它只控制取消；消息仍经 ready，统计结果在 join 后检查。
3. 要加入 close 并排空，随便加 closed bool？答：要定义关闭线性化时点、拒绝新写、已发布消息的读取和退出顺序；当前实现没有此功能。
4. 停止期间 pending unique_ptr 谁释放？答：所有端点停止后由 slot 的 payload 析构释放；这不是“只要设置取消标志就可以并行 free”。

### 链 F：能不能移到目标 MCU？

1. host flag_is_lock_free=1 足够吗？答：只报告这次宿主对象，目标原子实现和运行库另查。
2. 目标也是 lock-free 就是 ISR-safe？答：不够；嵌套、目标运行环境、可调用函数、T 的操作和进展约束都要评估。
3. noexcept callback 里能 delete、等待或获取锁吗？答：语言上可能，实时合同未必允许；本示例未给出允许 ISR 执行的保证。
4. DMA 不会调用 C++ store，release/acquire 会替它维护 cache 吗？答：不能从这份 C++ 线程实验推出；先确定设备完成、cache 一致性、内存区域和目标手册协议。此处没有 DMA/cache 实测。

## 10. 有界不等于硬实时，原子不等于目标无锁

SpscPublicationSlot 的 try 操作没有用户层重试循环，不使用队列 mutex；但泛型 T 的移动/析构/回调可以消耗任意时间，std::atomic 的实现也依目标而定。因此不把整个实现命名为“已证明 wait-free 队列”，更不宣称 ISR 安全。

[N4659 §32.6.1 [atomics.types.generic] ¶5–6](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/n4659.pdf#page=1364) 给出类型级 is_always_lock_free 和对象级 is_lock_free 查询。实测主机二者为 1，记录的是这个编译器、主机和 atomic<bool>；未测目标。按 [GCC 14.2 §6.57 __atomic Builtins](https://gcc.gnu.org/onlinedocs/gcc-14.2.0/gcc/_005f_005fatomic-Builtins.html)，不能生成无锁序列时可能调用外部运行库例程。这份编译器说明只用于界定需要查目标实现的原因，不证明所有 std::atomic 必然生成某条指令。

目标接入前的审查清单：

- 固定芯片/架构、C++ 运行环境、交叉编译器版本、标准库和链接配置。
- 对实际类型和存放地址核对无锁查询、对齐、生成代码和可能的运行库调用；不把宿主查询复制到报告。
- 清点生产/消费端是否任务、ISR、嵌套 ISR 或 DMA；每一端真正是否串行。
- 审查 T 及回调是否会分配、释放、阻塞、调用不允许的库函数或访问外部共享状态。
- 如果 DMA 参与，把设备可见性、cache 维护和完成通知加入单独合同；无硬件证据时保持“未验证”。
- 确定满时政策：丢弃、统计、重试、背压；确定关闭/复位政策。重试导致的等待不是 try 接口承诺的一部分。

本次没有执行上述目标步骤。这张清单是评审入口，不是已经完成的板级结论。

## 11. 验收标准与这次真实结果

运行验收必须同时满足：

1. 两个新程序严格编译，无警告降级；7 组运行检查全部通过。
2. 初空时 consume 和 visit 失败；output 不变，callback 调用次数为 0。
3. 容量精确为 1：第一次 publish 成功，未归还前第二次失败且 source 不变；只消费一次。
4. length=0…32 每种值、接近 UINT64_MAX 的 sequence 都完整传递；invalid length=33 由检查器拒绝。
5. 1000 次顺序回合，每次检查满拒绝、读出、空拒绝和复用；不会把正确性限制在第一轮。
6. unique_ptr 三件测试资源按已列明路径总共释放三次；满槽失败保持 source，移出对象在槽位复用后仍由消费者拥有。
7. 回调内的 32 次生产尝试全部失败且源保持；回调末尾 A 内容保持；回调返回后下一条 B 可传递。
8. 三种 yield 模式各 30000 条，总 90000 条顺序号和全部字段正确；结束后槽位为空，失败输出保持。
9. 三个不满足类型/接口合同的编译样例被拒绝：抛异常移动赋值、非 noexcept 回调、复制槽位。只做编译，不执行它们。
10. ASan/UBSan 与普通结果分开记录，环境失败和未运行阶段不能写成通过；泄漏关闭必须显式说明。

2026-10-11 实测：GCC g++ 14.2.0（Debian 14.2.0-19）；普通入口通过；ASan/UBSan 在显式 detect_leaks=0 后通过同一批新程序和测试。默认泄漏检测在 demo 退出阶段报告 LeakSanitizer fatal error 和 ptrace 不支持，导致默认 sanitizer 入口失败，后续 ownership 与编译拒绝阶段未运行；这个失败记录完整保留。**没有泄漏检测通过结论。** 计数资源的精确释放验收不能替代 LSan。

ASan/UBSan 检查实际执行轨迹中的相应内存/未定义行为问题，不是线程数据竞争证明。未运行 TSan、Clang、形式模型检查器、旧项目全套测试、目标交叉编译/反汇编、ISR/RTOS/DMA/cache/WCET 检查，也没有执行故意竞态版本。检测范围核对 [GCC 14.2 Instrumentation Options](https://gcc.gnu.org/onlinedocs/gcc-14.2.0/gcc/Instrumentation-Options.html) 的 -fsanitize=address、-fsanitize=undefined 与 -fno-sanitize-recover 说明。

线程创建之前构造槽位，成功 join 后读取线程独占统计，是本实验生命周期闭环的一部分。相关同步依据见 [N4659 §33.3.2.2 [thread.thread.constr] ¶6 与 §33.3.2.5 [thread.thread.member] ¶3–4](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/n4659.pdf#page=1379)。yield 只是调度提示，不作为同步证明，核验点是 [N4659 §33.3.3 [thread.thread.this] ¶2–3](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/n4659.pdf#page=1381)。

## 12. 白板自检与进一步练习

无需改代码就先完成：

- 指出三个普通对象：payload、producer source、consumer output。给每个对象标谁能访问及访问窗口。
- 不看答案画 Wk→Rk→Wk+1；分别圈出 Pk/Ak 和 Fk/Ek+1；漏一条边就重画。
- 用“第一次初始化、发布、归还”三类 store 解释反复出现的 false/true，不能只画两个值。
- 分别说明“payload 活着”“此刻我有访问许可”“T 管理资源由我释放”，每句都配上代码位置。
- 检查一个含 string_view 的类型：移出之后 view 指向谁的存储？如果仍指向槽位，能否在归还后继续用？

增加功能前先写新合同：支持可抛异常 T、立即 dispose、close-and-drain、多生产者、环容量、在途取消。每项都改变本章的某个前提；不要仅添加字段后延用旧证明。仅练习时也不能执行故意数据竞争版本。

## 13. 固定来源、核验点与原创边界

详尽列表在 [source checklist](../../sources/publication-ownership-verification.json)。来源只用于核对规则；算法、白板推导、场景、问题链和测试是本次原创，不摘录平台面经，不声称来源作者背书。

- 固定标准依据：[N4659（2017-03-21）](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/n4659.pdf)。核心：§4.7.1、§32.4、§32.6.1、§23.11.1.2、§33.3.2.2/5；使用固定节标签和段号，避免引用滚动草案的后续语义。
- 固定编译器文档：[GCC 14.2 Volatiles](https://gcc.gnu.org/onlinedocs/gcc-14.2.0/gcc/Volatiles.html)、[__atomic Builtins](https://gcc.gnu.org/onlinedocs/gcc-14.2.0/gcc/_005f_005fatomic-Builtins.html)、[Instrumentation Options](https://gcc.gnu.org/onlinedocs/gcc-14.2.0/gcc/Instrumentation-Options.html)。说明实现/检测边界，不替代标准或目标手册。
- 本章只把官方一手材料作为技术依据；没有把 GitHub 评论、牛客题解、小红书笔记或知乎回答当成 C++ 内存模型权威。
