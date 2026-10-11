# 可选扩展  GUI 存储网络与媒体的集成边界

学习优先级：P2，随 Linux 应用项目选读。本篇在 [LX01–LX18 的系统基础](lx01.md)之后，把可选库接成有边界的应用链，不新增核心题号。无需先安装全部库；选项目真正需要的一种 UI、一种存储或一种数据通路，先验证它与核心模块之间的接口。

## 学习目标与前置知识

一个显示、记录和上传采集结果的终端，至少有 UI 事件循环、业务处理、存储和网络拥有者。加入媒体后还会出现采集池、编码解码和时间同步。库能处理各自的机制，但不会自动决定“结果属于哪个页面”“写入是否持久”“消息对应的动作是否完成”“关闭是否仍有回调”。

先读 [LX02 的 I/O 返回值](lx02.md)、[LX04 的同步谓词](lx04.md)、[LX09 的停止与回收](lx09.md)、[LX17 的有界任务](lx17.md)、[PR08 的幂等](../60-protocols/pr08.md)、[PR11 的 TLS](../60-protocols/pr11.md)与[PR13 的 HTTP](../60-protocols/pr13.md)。性能和故障工具继续使用 [LX08](lx08.md)、[DB02](../100-debugging/db02.md)、[DB04](../100-debugging/db04.md)与 [DB05](../100-debugging/db05.md)的证据方法；本篇不另造一套 gdbserver、ASan、Valgrind、perf、strace 或 syslog 教程。

## 一  用一份边界表组织应用

以虚构的采集预览终端为例，UI 发起“打开来源”，worker 产生带序号的结果；本地记录和网络上传各自确认，媒体展示只保留足够新鲜的帧。先把交界处写清楚：

| 交界处 | 数据与拥有者 | 完成与失败含义 |
| --- | --- | --- |
| UI → worker | 命令值、请求 ID、页面代次、deadline | 已接受不等于执行完成；队满需明确返回 |
| worker → UI | 不可变快照或明确共享拥有的结果 | 旧代次不更新新页面；结果入口有容量上限 |
| 业务 → SQLite | 连接/事务拥有者，参数化值 | COMMIT 结果与配置决定数据库状态，不替代设备动作完成 |
| 业务 → HTTP/MQTT | 请求/业务 ID、大小与离线预算 | 传输确认、服务确认、业务完成分别记状态 |
| 媒体 → 消费者 | 格式、stride、时间基、引用和释放责任 | EOF/EOS、暂时缺输入、取消和错误分别处理 |

这些是本篇建议的应用合同，不是任何库默认保证。一个统一的“成功”布尔会丢掉关键状态，宜使用明确的 accepted、running、completed、cancelled、failed 等状态及可核对的错误原因。

## 二  GUI 线程只接收有界结果

Qt 与 LVGL 都能用于某些嵌入式界面，但其对象、线程、显示后端和资源要求不同。不要把二者的线程规则互相套用，也不要仅因设备运行 Linux 就假定显示、输入和 GPU 插件已经可用。

Qt 6.8 的 [QObject 线程规则](https://doc.qt.io/qt-6.8/threads-qobject.html)要求区分对象的线程归属与调用发生的线程。GUI 对象应由 GUI 线程访问；queued connection 把槽调用交给接收者的事件循环，direct connection 在发射者线程执行，名称里有 slot 不代表自动切回 UI。接收线程没有运行事件循环时，排入调用也不会自动得到服务。

LVGL v9.1.0 的[操作系统移植说明](https://github.com/lvgl/lvgl/blob/e1c0b21b2723d391b885de4b2ee5cc997eccca91/docs/porting/os.rst)说明默认不线程安全。一个简单的应用策略是让单一 GUI 拥有者调用一般 UI API，worker 只投递数据；若使用多任务调用，必须按该版本的例外和统一锁规则覆盖相关 API 及 timer handler。不能只锁自己的 widget 指针而漏掉库内部状态。

耗时查询、解码、推理或网络等待放在 worker 中，但 UI 接口仍需要自己的背压。queued signal 本身不是应用定义的“最多 N 条结果”合同。对于预览可采用“有界 mailbox + 一个待处理唤醒”，把连续更新合并为最新快照；命令结果不能悄悄覆盖，应保留请求 ID 和明确的拒绝或取消通知。

假设页面 A 发起请求 17，用户随后打开页面 B。返回值必须同时检查请求 ID 与页面代次，即使 A/B 恰好显示同一设备名称也不能省略。关闭页面时先取消订阅和新任务，再等所有可能触及该页面的调用退出。传原始对象指针让后台任务自行更新 widget，会把结果寿命与页面寿命纠缠在一起。

验收至少覆盖连续点击、切页后旧结果到达、worker 失败、UI 消费慢、关闭时任务仍运行。记录 mailbox 峰值、合并/拒绝数量和最终释放数。线程创建、锁与 join 的实现仍见 [LX04](lx04.md)、[LX09](lx09.md)和 [LX17](lx17.md)。

## 三  SQLite 的事务边界与耐久边界

本地存储先确定哪些数据必须一起成功。例如“收到业务 ID → 更新记录 → 保存应答”可以放入同一个事务；任何一步失败时按明确策略回滚。参数化 SQL 使数据与 SQL 语法分离，不拼接外部字符串生成语句。

SQLite 允许多个读事务，但同一数据库同时只有一个写事务。BEGIN IMMEDIATE 可在进入多步写逻辑前请求写事务，仍可能返回 SQLITE_BUSY。长事务中等网络、等用户或做大计算，会拉长其他写入的等待。busy timeout 是等待策略，不是增加写者数量，也不是应用总 deadline；重试预算仍由应用决定。[官方事务说明](https://www.sqlite.org/lang_transaction.html)可核对事务、错误后状态和 COMMIT 失败的行为。

连接、prepared statement 和活动 cursor 要有明确拥有者。即使采用线程安全构建，也不能让多个线程的语句任意交错成同一个业务事务。可采用单一存储 worker 串行执行短事务，其他模块通过有界请求/结果传递。此策略仍需处理取消前已提交、磁盘满、I/O 错误与模式升级失败。

写入返回、事务提交、掉电后耐久与 Flash 寿命是不同验收。保存 journal_mode、synchronous、page/cache 设置、checkpoint 策略、VFS、文件系统和介质。WAL 可以改善读写并发，但仍只有一个写者；长读事务会阻碍 checkpoint 进展并让 WAL 增长。WAL 中使用 NORMAL 时，部分已提交事务在掉电或硬重启后可能回退，不能当作 FULL 的同等耐久保证；真实耐久仍依赖存储与同步实现。[官方 WAL 说明](https://www.sqlite.org/wal.html)用于核对这些条件。

WAL 数据库的复制与备份不能只抓主数据库文件，忽略仍有有效状态的 WAL。实际工程用受支持的备份方法，并验证恢复；不要自行删除活动数据库的 -wal/-shm 文件来“清空间”。文件映射和持久性基础见 [LX05](lx05.md)。

版本基线选 [SQLite 3.51.3](https://www.sqlite.org/releaselog/3_51_3.html)，该发布说明明确修复 WAL-reset 数据库损坏问题。本篇不将旧版作为生产推荐；已有系统应核对实际 SQLite 版本和厂商是否带入对应修复。应用单元测试未触发错误，不能证明它不受某个已知竞争问题影响。

## 四  HTTP 与 MQTT 的确认到哪一层

HTTP 上传可使用 libcurl，但库句柄也需要拥有者。curl 8.8.0 的[线程说明](https://github.com/curl/curl/blob/fd567d4f06857f4fc8e2f64ea727b1318f76ad33/docs/libcurl/libcurl-thread.md)不允许同一 handle 同时被多个线程使用；share 接口涉及另外的锁规则。为一次请求规定连接和总时间预算、响应最大大小、取消、重试与业务 ID。HTTP 状态码、传输成功与服务器业务结果仍应分别检查，方法和重试语义见 [PR13](../60-protocols/pr13.md)。

OpenSSL 是可选 TLS 实现之一，libcurl 是否使用它取决于构建和运行配置。记录实际 TLS 后端、可信根、域名校验、时间、协议策略及错误阶段，不靠“链接了 OpenSSL”宣称通信安全。没有完成真实 TLS 联网时如实写未运行，不能以关闭验证作为测试通过。机制见 [PR11](../60-protocols/pr11.md)。

MQTT 的 QoS 描述协议交付，不描述电机、数据库或其他业务动作的提交。MQTT 5.0 的 QoS 0/1/2 分别定义至多一次、至少一次和协议范围内的恰好一次交付；QoS 2 的确认交换不能自动把外部设备动作与应用去重记录原子提交。客户端即使选择不处理 Application Message，也必须遵守收到 PUBLISH 的适用 QoS 确认规则；QoS 0 本身不发送协议应答。[OASIS MQTT 5.0](https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html)的 §4.3、§4.5 用于核对这些边界。

业务 ID 必须与可重用的 MQTT Packet Identifier 分开。给命令保存 issuer、业务 ID、参数摘要、有效期和结果；同 ID 不同参数应拒绝或报警，不能返回旧结果装作成功。去重表应有保留期限和容量，和允许的重试/离线窗口一致。

若动作是数据库内的更新，去重记录和更新可放在同一事务中。若动作是外部设备控制，数据库提交与实际动作之间存在崩溃窗口：先记完成可能漏执行，先执行再记完成可能重复。可按业务采用绝对状态设置、设备端序号、可查询结果或人工处理不确定状态；不能用一个本地 UNIQUE 约束就宣称跨设备恰好执行一次。基础见 [PR08](../60-protocols/pr08.md)。

离线需分别限制客户端待发、broker 会话队列、在途消息、应用待处理和去重历史。保存数量、字节、年龄与满队策略，以及重连、Session Expiry、Message Expiry、订阅和 retained 消息策略。retained 消息不是任意历史日志；过期控制命令不能因重连就重新执行。Mosquitto v2.0.18 的[配置说明](https://github.com/eclipse-mosquitto/mosquitto/blob/3923526c6b4c048bbecad2506c4c9963bc46cd36/man/mosquitto.conf.5.xml)可核对 max_queued_messages/max_queued_bytes 和 persistence；broker 的持久化并不替代应用业务记录。确认目标版本的溢出行为并用计数/业务应答观测丢失，不能默认所有库都把丢弃显式返回给发送者。

## 五  媒体数据还带时间和引用

媒体链比“读字节 → 处理”多了格式协商与时间语义。写清分辨率、像素/采样格式、plane 与 stride、颜色信息、时间基、PTS/DTS、缺失或跳变 timestamp 的策略。解码顺序和展示顺序可能不同；不要把每个压缩 packet 当成一个显示 frame，也不要把 PTS 直接当本机单调时钟。

FFmpeg n7.0 的 [send/receive 接口](https://github.com/FFmpeg/FFmpeg/blob/083443d67cb159ce469e5d902346b8d0c2cd1c93/libavcodec/avcodec.h)把输入与输出分开：一次输入可能产生零个、一个或多个输出；EAGAIN 要按接口状态推进另一侧，不能只 sleep 原地等。正常输入结束时发送 drain 标记并取剩余输出到 EOF。取消则可以选择丢弃剩余工作，这与正常排空是两种产品策略。[AVFrame 定义](https://github.com/FFmpeg/FFmpeg/blob/083443d67cb159ce469e5d902346b8d0c2cd1c93/libavutil/frame.h)用于核对时间与引用字段。

GStreamer 1.24.5 的 [queue 源码及接口说明](https://github.com/GStreamer/gstreamer/blob/3c66f10e218ec93a11e93f7c0bd209036c867a7a/subprojects/gstreamer/plugins/elements/gstqueue.c)提供 buffers/bytes/time 上限、阻塞与 leaky 策略，信号可能来自 streaming thread。把这些信号送 UI 时仍要遵守 UI 线程边界。不能只在源码里看到默认值，就假定整条 pipeline 总内存或总延迟有同样上界；其他 element、appsink、解码重排和驱动也可持有 buffer。

预览一般关注帧的新鲜度，录像一般关注完整性，算法可能要求连续时间窗口。决定丢弃位置前先判断是否会破坏编解码依赖；随意丢压缩 packet 与丢已解码的独立预览帧不同。队列时间字段和端到端捕获到显示延迟也不是同一指标。

媒体 buffer 可被 refcount、池或驱动拥有。队列传递对象时，必须增持引用或转移拥有权；消费者完成后释放，旧引用清零才能复用。可参考固定版本的 [GstBuffer](https://github.com/GStreamer/gstreamer/blob/3c66f10e218ec93a11e93f7c0bd209036c867a7a/subprojects/gstreamer/gst/gstbuffer.h)。零拷贝路径还要核对设备格式、映射、同步和缓存规则，不能仅因共享一个指针就宣布零拷贝正确。

停机顺序应先拒绝新输入，再按策略排空或取消，唤醒阻塞者，等待 callback/streaming worker 收敛，最后释放 pipeline、池、输出和 UI 对象。EOS、flush、错误和取消分别设置终态；[GStreamer 1.24.5 的事件定义](https://github.com/GStreamer/gstreamer/blob/3c66f10e218ec93a11e93f7c0bd209036c867a7a/subprojects/gstreamer/gst/gstevent.h)可核对 EOS 与 flush 的区别。正常文件结束可以 drain，拔掉采集设备或用户强制关闭不一定应继续等待全部旧帧。

## 六  版本许可与运行配置一起保存

依赖清单不仅列库名，还要列准确版本/commit、启用模块、构建选项、动态依赖、插件路径、显示后端、codec、TLS 后端、目标 ABI、许可证和第三方组件。文档基线与实际执行版本分别记录。设备 rootfs 升级后不能只保留应用源码哈希。

| 文档基线 | 可复查的许可或选项入口 |
| --- | --- |
| Qt 6.8；qtbase v6.8.0 | [官方许可说明](https://doc.qt.io/qt-6.8/licensing.html)：商业、LGPLv3/GPLv3 路径及模块差异，第三方组件另列 |
| LVGL v9.1.0 | [MIT 文本](https://github.com/lvgl/lvgl/blob/e1c0b21b2723d391b885de4b2ee5cc997eccca91/LICENCE.txt)；显示/输入 port 与外部驱动另核对 |
| SQLite 3.51.3 | [官方 public domain 说明](https://www.sqlite.org/copyright.html)；扩展和构建工具分别检查 |
| curl 8.8.0；OpenSSL 3.0.14 | [curl 许可](https://github.com/curl/curl/blob/fd567d4f06857f4fc8e2f64ea727b1318f76ad33/COPYING)、[OpenSSL Apache-2.0 文本](https://github.com/openssl/openssl/blob/9cff14fd97814baf8a9a07d8447960a64d616ada/LICENSE.txt)；TLS 后端由实际构建决定 |
| Mosquitto v2.0.18 | [EPL-2.0/EDL-1.0 双许可入口](https://github.com/eclipse-mosquitto/mosquitto/blob/3923526c6b4c048bbecad2506c4c9963bc46cd36/LICENSE.txt)；客户端与 broker 配置分开 |
| FFmpeg n7.0 | [许可与构建选项说明](https://github.com/FFmpeg/FFmpeg/blob/083443d67cb159ce469e5d902346b8d0c2cd1c93/LICENSE.md)：LGPL 路径、可选 GPL 和 nonfree 条件须依实际构建核对 |
| GStreamer 1.24.5 | [核心 LGPL 文本](https://github.com/GStreamer/gstreamer/blob/3c66f10e218ec93a11e93f7c0bd209036c867a7a/subprojects/gstreamer/COPYING)；每个插件和外部 codec 的许可另查 |

此表不提供法律结论，不等于这些组合在某块板上获支持，也不建议忽略后续安全更新。分发前按真实组件、链接/分发方式、模型与媒体素材逐项核对条款，疑难情况交由合适的专业人员判断。固定版本有助复查；它不能替代维护、漏洞与已知问题检查。

## 七  一个本地事务与重复请求模型

下例只演示数据库内的“业务 ID 去重 + 数据更新 + 保存结果”同事务完成。故意让更新后抛错，检查整个事务回滚；随后重复同一 ID，检查没有再次修改本地值。没有 MQTT broker、TLS、GUI、媒体或外部设备动作。

```python
import sqlite3

db = sqlite3.connect(":memory:", isolation_level=None)
db.execute("CREATE TABLE seen(id TEXT PRIMARY KEY, value INTEGER NOT NULL)")
db.execute("CREATE TABLE state(value INTEGER NOT NULL)")
db.execute("INSERT INTO state VALUES(0)")

def apply(operation_id, value, fail=False):
    db.execute("BEGIN IMMEDIATE")
    try:
        prior = db.execute("SELECT value FROM seen WHERE id=?", (operation_id,)).fetchone()
        if prior is not None:
            if prior[0] != value:
                raise ValueError("same ID with different payload")
            db.execute("COMMIT")
            return "duplicate"
        db.execute("UPDATE state SET value=?", (value,))
        if fail:
            raise RuntimeError("injected before dedup record")
        db.execute("INSERT INTO seen VALUES(?, ?)", (operation_id, value))
        db.execute("COMMIT")
        return "applied"
    except Exception:
        if db.in_transaction:
            db.execute("ROLLBACK")
        raise

try:
    apply("request-1", 7, fail=True)
except RuntimeError:
    pass
assert db.execute("SELECT value FROM state").fetchone() == (0,)
assert db.execute("SELECT COUNT(*) FROM seen").fetchone() == (0,)
assert apply("request-1", 7) == "applied"
assert apply("request-1", 7) == "duplicate"
db.close()
print("local transaction model passed", sqlite3.sqlite_version)
```

本次在 Python 3.12.14、其实际链接的 SQLite 3.53.1 上执行此示例，未安装或执行文档基线 SQLite 3.51.3。补充主机测试覆盖同 ID 参数冲突、事务回滚、两个连接的写锁争用、只保留最新结果的代次模型，以及简化媒体队列的上限、排空和取消。未做掉电、Flash 耐久、WAL-reset 复现、真实 MQTT/HTTP/TLS、Qt/LVGL、FFmpeg/GStreamer 或板级测试。

## 八  练习与验收

选择一条需要的通路，例如“采集 → 后台处理 → UI”，再加“存储 → 上传”。不要一次堆上所有库。

1. 写拥有者、数据上限、时间上限、完成含义和停机顺序。
2. 验证切页旧结果、慢消费者、满队列、重复 ID 与重启后的业务恢复。
3. 对存储测试事务中间失败、锁忙和空间不足；掉电与耐久另列真实设备计划。
4. 对网络分开记录库接收、协议确认、业务确认，验证离线积压和过期。
5. 对媒体验证非一对一输入输出、时间跳变、EOS 排空和用户取消；记录捕获到消费者的实际延迟。
6. 保存实际依赖/插件/许可清单，输出每项通过、失败或未运行的证据。

验收目标是每个交界都有可重放的失败行为和明确终态。工具能证明的范围以 [DB04](../100-debugging/db04.md)为准，资源和性能证据沿用 [LX08](lx08.md)。能解释跨模块合同，比在项目介绍中列一长串库名更有用。

正文、边界表、接口策略与主机例子为原创教学内容。公开官方来源用于核对机制和版本，不表示第三方为本手册背书。
