# socket 生命周期与 WebSocket 消息合同

学习优先级：P2，可选补充。先读 [PR05 分层](../60-protocols/pr05.md)、[PR06 字节流定界](../60-protocols/pr06.md)、[LX02 部分 I/O](lx02.md) 和 [LX18 就绪与公平预算](lx18.md)。本页补齐端点生命周期与协议/API 的区别，不另建核心题号，也不重新讲完整 TCP 或事件循环。

学习目标是能为一个端点说明：谁创建和关闭它、读写结果意味着什么、队列何时拒绝、何时才算业务完成。正文、推演和练习均为原创；禁止照搬、拼接或冒名发布他人的课程文字、代码和图表。公开规范只用于核对机制，不能把规范示例当自己的工程成果。

## 直接回答

socket 是操作系统提供的通信端点/API 抽象；TCP、UDP 是传输协议；WebSocket 是带握手和消息帧的应用协议；浏览器 WebSocket 对象是受限制的高层 API。它们可以出现在同一条路径上，不是相互替换的同层产品。选择协议后，仍要设计 fd 所有权、资源上限、超时、重连和应用回执。

## 一 先把层次和角色讲清楚

`socket(domain, type, protocol)` 选择通信域、通信语义和协议并返回 fd。AF_UNIX 可用于本机进程，AF_INET/AF_INET6 用于相应 IP 地址族；不能看到 socket 就断言跨机器或 TCP。[socket(2)](https://github.com/mkerrisk/man-pages/blob/ae6b221882ce71ba82fcdbe02419a225111502f0/man2/socket.2)

| 名称 | 本页讨论的责任 | 不提供的保证 |
|---|---|---|
| TCP stream socket | 用 fd 操作有序字节流 | 一次 recv 等于一条业务消息 |
| UDP datagram socket | 收发独立数据报及对端地址 | 交付、顺序和业务执行 |
| WebSocket 协议 | 握手、文本/二进制消息与控制帧 | 命令的幂等执行和应用回执 |
| 浏览器 WebSocket API | 暴露连接状态和消息事件 | 任意原生 socket 调用能力 |

客户端/服务器描述一次服务关系：客户端发起请求，服务器提供服务。P2P 描述参与者也可向其他参与者提供资源；实际系统可混合中心发现与点对点数据路径。P2P 不意味着不用服务器、端点都同等可信或没有资源限制。[RFC 5694 §2、§4](https://www.rfc-editor.org/rfc/rfc5694.html)

原创选择例：十台采样设备只向一个汇总端上传时，可把发现、认证和连接预算集中管理。若设备还互相交换缓存片段，每台设备就新增服务者责任：发现哪些同伴、验证哪些数据、同时服务几个同伴、缓存最多多少。这里最多服务两个同伴是示例预算，不能由“P2P”三个字推出性能或可靠性结论。

## 二 TCP 监听端点与连接端点分开拥有

| 阶段 | 成功后的对象与下一步 | 本应用需补的合同 |
|---|---|---|
| socket | 尚未用于监听或连通的端点 fd | 失败则无 fd 可交给 worker |
| bind | 指定本地地址 | 地址族、长度与端口必须合法；失败不能继续 listen |
| listen | 被动接受连接的监听端点 | 连接预算与待 accept 队列是不同预算 |
| accept/accept4 | 一个新的连接 fd；原监听 fd 保留 | 成功即确定新 fd 的拥有者与关闭路径 |
| 客户端 connect | 成功后可进行连接 I/O | 失败/取消/超时必须收敛到释放资源 |

bind 指定本地地址，connect 指定对端。IP 客户端若不要求固定本地地址/端口，常由内核选择；不要机械要求每个客户端先显式 bind。[bind(2)](https://github.com/mkerrisk/man-pages/blob/ae6b221882ce71ba82fcdbe02419a225111502f0/man2/bind.2)、[connect(2)](https://github.com/mkerrisk/man-pages/blob/ae6b221882ce71ba82fcdbe02419a225111502f0/man2/connect.2)

accept 产生新的已连接 socket，既不把监听 fd 变成业务 fd，也不取消监听。Linux 不保证新 fd 继承监听端的 O_NONBLOCK；可用适用的 accept4 标志明确设置 SOCK_NONBLOCK/SOCK_CLOEXEC。监听端也需非阻塞，才能在就绪状态变化时避免意外挂住。[accept(2)](https://github.com/mkerrisk/man-pages/blob/ae6b221882ce71ba82fcdbe02419a225111502f0/man2/accept.2)

Linux TCP 的 listen backlog 限定等待应用 accept 的已建立连接队列，并受内核上限约束；它不是本应用全部活跃连接的最大数量。[listen(2)](https://github.com/mkerrisk/man-pages/blob/ae6b221882ce71ba82fcdbe02419a225111502f0/man2/listen.2)

原创所有权例：监听器拥有 L，连接对象拥有 C1/C2。关闭 C1 不影响 L 继续接入；关闭 L 不等于关闭已经接入的 C2。连接上限到了就暂停接入或明确拒绝，不能 accept 成功后因分配失败把新 fd 遗忘。连接对象用递增代号区别复用的同一个 fd 数字；详情见 [LX14](lx14.md)。关闭前撤销异步引用；Linux close 报错后不能盲目再次 close 同一个整数，否则可能关闭已复用的对象。[close(2)](https://github.com/mkerrisk/man-pages/blob/ae6b221882ce71ba82fcdbe02419a225111502f0/man2/close.2)

非阻塞 IP TCP connect 返回 EINPROGRESS 表示建连尚未结束。等待可写后读取 SOL_SOCKET/SO_ERROR，零才是成功；可写本身不是成功。用单调时钟的总 deadline 控制等待。建连失败后采用关闭旧端点、创建新端点的保守恢复路径，不在状态不明的 fd 上无限 connect。[connect(2)](https://github.com/mkerrisk/man-pages/blob/ae6b221882ce71ba82fcdbe02419a225111502f0/man2/connect.2)

## 三 UDP 接收的是数据报，不是较快的 TCP

典型服务端是 socket→bind→recvfrom/recvmsg→sendto；UDP 没有 TCP 式 listen/accept。客户端可直接 sendto，让内核自动选择本地端口；需要稳定接收地址时再明确 bind。Linux UDP 的 connect 设置默认发送对端及接收对端筛选，不进行 TCP 式握手，也没有因此增加交付保证。[UDP 接口](https://github.com/mkerrisk/man-pages/blob/ae6b221882ce71ba82fcdbe02419a225111502f0/man7/udp.7)、[RFC 768](https://www.rfc-editor.org/rfc/rfc768.html)

接收合同要分清三个结果：

- 一次接收消费一个数据报。缓冲过小会截断，余下内容不能靠下一次 recv 拼回；使用 recvmsg 检查输出 MSG_TRUNC，按协议拒绝整份截断数据。[recv(2)](https://github.com/mkerrisk/man-pages/blob/ae6b221882ce71ba82fcdbe02419a225111502f0/man2/recv.2)
- 零长度数据报是合法消息。UDP 返回零不能照搬 stream 的有序 EOF 语义；本页实验使用 recvmsg/recvfrom 消费它。[recv(2)](https://github.com/mkerrisk/man-pages/blob/ae6b221882ce71ba82fcdbe02419a225111502f0/man2/recv.2)
- recvfrom/recvmsg 返回来源地址；C 接口每次调用前重新给地址长度容量。回复使用该次合法请求的来源，并先验证长度、版本和业务允许的对端。来源 IP/端口或 connected UDP 筛选不是身份认证。[recv(2)](https://github.com/mkerrisk/man-pages/blob/ae6b221882ce71ba82fcdbe02419a225111502f0/man2/recv.2)、[connect(2)](https://github.com/mkerrisk/man-pages/blob/ae6b221882ce71ba82fcdbe02419a225111502f0/man2/connect.2)

不要把 TCP 的“短写后发送剩余字节”循环直接套在一个 UDP 消息上：一次数据报发送应整体成功或按错误处理；EMSGSIZE 要重新评估数据报上限，不把剩余部分偷偷变成第二条业务消息。默认不使用会改变合并语义的 UDP_CORK/MSG_MORE。[send(2)](https://github.com/mkerrisk/man-pages/blob/ae6b221882ce71ba82fcdbe02419a225111502f0/man2/send.2)

原创例：协议上限 64 字节，收到 90 字节的数据报且截断标志置位，计数并丢弃这一报；随后独立接收的 3 字节消息不能拿来补齐它。重试、请求 ID、迟到处理和拥塞友好预算见 [PR08](../60-protocols/pr08.md)。

## 四 部分 I/O 必须留在连接状态里

TCP 每次 send/recv 的返回值只解释本次进度：正值推进 offset，EINTR 保留进度并受总 deadline 约束，EAGAIN/EWOULDBLOCK 回到等待，其他错误走停止/回收。非零接收容量下，stream recv 返回零表示对端有序结束发送；已收的完整帧可以处理，不完整帧应报告截断。半关闭机制见 [PR07](../60-protocols/pr07.md)。发送到已关闭 stream 可出现 EPIPE/SIGPIPE，Linux 可按应用合同使用 MSG_NOSIGNAL。[send(2)](https://github.com/mkerrisk/man-pages/blob/ae6b221882ce71ba82fcdbe02419a225111502f0/man2/send.2)、[recv(2)](https://github.com/mkerrisk/man-pages/blob/ae6b221882ce71ba82fcdbe02419a225111502f0/man2/recv.2)

这里不再提供第二个帧解析器。把 [PR06](../60-protocols/pr06.md) 的有界 parser 接到连接对象，让头部/主体进度跨回调保存。每连接再明确最大待发字节、最大未确认命令数、空闲/收帧/业务 deadline；ET 的预算用尽后仍可继续工作的状态见 [LX18](lx18.md)。

## 五 WebSocket 的协议边界

RFC 6455 的 HTTP/1.1 路径以 GET Upgrade 握手开始，验证 101 与相应升级字段、Sec-WebSocket-Accept，按需协商子协议/扩展。校验 Accept 表明对方理解握手，不代替 TLS 身份验证或业务认证。一条消息可跨多帧；文本是 UTF-8，二进制内容由业务定义。客户端到服务器的帧必须 masking，服务器到客户端不 masking；masking 不是加密，wss 的保密/完整性来自安全传输。控制帧不可分片且负载不超过 125 字节，可穿插于分片消息中；Ping/Pong 不是业务完成回执。Close 启动关闭交换，收到 Close 后按状态处理并设关闭期限；断链不必然收到正常 Close。实现需限制帧和累计消息大小、校验长度/保留位/协商扩展并拒绝非法输入。[RFC 6455 §4–§8、§10](https://www.rfc-editor.org/rfc/rfc6455.html)

这段只划定实现检查项，不给自制安全解析器或完整握手算法。教学协议模型不是互操作认证，生产库/代理/浏览器也需针对选定版本验证。

WebSocket 不能被概括为“总是 HTTP/1.1 101 后独占一条 TCP”。RFC 8441 用 HTTP/2 Extended CONNECT 在 stream 上承载它；RFC 9220 给出 HTTP/3 的对应路径，HTTP/3 使用 QUIC。握手映射与关闭底层 stream 的方式因路径而异；这些扩展保留 WebSocket 消息/帧责任，不能把 HTTP/1.1 升级头原样套到 HTTP/2/3。[RFC 8441 §3–§5](https://www.rfc-editor.org/rfc/rfc8441.html)、[RFC 9220 §3](https://www.rfc-editor.org/rfc/rfc9220.html)

Origin 检查、认证、每操作授权和资源预算要同时成立。非浏览器客户端可自行构造 Origin，因此 Origin 不是身份凭据；不能靠“来自允许网页”就执行任意设备命令。[RFC 6455 §10.1–§10.2](https://www.rfc-editor.org/rfc/rfc6455.html)

## 六 浏览器 API 不暴露原生 fd

经典浏览器 WebSocket 构造参数是 URL 和可选子协议，脚本获得 readyState、send、message/close 等事件，而不是 bind/listen/accept 或原始 UDP 接口；构造器也没有任意自定义握手 header 参数。消息事件不是一次 TCP recv，也不保证一次网络包对应一次事件。bufferedAmount 表示尚未发送的应用数据量，归零不证明对端应用处理完成，更不为接收侧提供业务处理能力的保证。[W3C WebSocket API 2012-09-20 §4、§8](https://www.w3.org/TR/2012/CR-websockets-20120920/)

这里固定引用经典 API 的历史规范快照，只讲上述接口边界；不据此承诺现代浏览器、扩展或替代 API 的兼容性。原生客户端可拥有更多 socket/TLS 配置能力，但也要自行承担相应状态与认证责任。

## 七 原创有界发送与重连推演

假设课堂客户端每条消息最多 256 字节，待发队列最多 16 条且总共最多 4096 字节；只有 OPEN 状态能交给传输适配器。队满时遥测可合并为“最新值”，控制命令则明确拒绝提交。该策略属于应用设计，协议不会自动替你选择。

给每条命令记录 operation_id、会话代号、过期时刻和状态：待发送→传输已接受→应用回执完成，或明确失败/结果未知。断链时保留“结果未知”，不能改写成“服务器没执行”。重连创建新会话，采用有上限退避和抖动；是否重发取决于命令是否仍有效、服务端是否具有对应去重/查询合同。只收到 Ping/Pong、TCP ACK 或队列清空都不能推进到应用回执完成。

示例账本：提交 18 条、接受 16 条、拒绝 2 条；先发 5 条只收到 3 份业务回执后断链，便是 3 条完成、2 条结果未知、11 条未发送。测试必须保留这个区分，不宣称“重连后自动 exactly once”。

## 练习与验收

1. loopback 建连并 accept，关闭监听 fd 后继续验证已接入 fd 的收发；成功后关闭全部对象。
2. UDP loopback 收零长度消息、来源地址和截断报文，验证下一报不被拿来补齐前一报。
3. 非阻塞 socketpair 制造 EAGAIN 和短写，检查进度保存、整体期限及字节一致性。
4. 用纯主机账本模型注入队满、断链、过期和重复回执，检查消息数与内存预算。

上述实验只证明当前主机的 loopback/socketpair 与模型合同。它们没有验证远端网络、NAT/代理、TLS、真实浏览器 WebSocket、HTTP/2/3 互操作或 MCU 网络栈。本页也未覆盖所有 socket 类型和网络协议。

## 来源与适用范围

- Linux man-pages：上文固定到公开维护者仓库快照 `ae6b221882ce71ba82fcdbe02419a225111502f0`，逐页核对 API 合同。它不是最新版本声明；部署时再对照目标内核/libc。
- [RFC 768（1980）](https://www.rfc-editor.org/rfc/rfc768.html)、[RFC 9293（2022）](https://www.rfc-editor.org/rfc/rfc9293.html) 作为 UDP/TCP 协议基线；socket API 不由这两份 RFC 定义。
- RFC 6455（2011）、RFC 8441（2018）、RFC 9220（2022）：分别限定基础 WebSocket 与 HTTP/2、HTTP/3 承载。
- RFC 5694（2009）是 P2P 架构分析的 Informational 文档，不是要求采用 P2P 的标准。
- W3C WebSocket API（2012-09-20 Candidate Recommendation）是有日期的历史快照，不是最新版兼容性声明。

正文与练习不复制这些来源的完整算法、代码或图表；来源作者与机构不为本页工程设计背书。
