# 可选扩展  从算法输入到目标设备结果的部署验证

学习优先级：P2，随项目需要选读。本篇补充 [PJ05 的硬件与软件数据链](pj05.md)和 [PJ03 的项目证据](pj03.md)，不新增核心题号。先完成语言、所有权、构建及所选 MCU 或 Linux 主线，再选一个算法、一种目标设备和一条输入输出链。

## 学习目标与范围

部署完成应有三组证据：同一输入的结果符合合同，实际目标的资源和时间预算满足要求，失败与恢复不会破坏系统。导出文件、转换成功、桌面上运行一次，只能关闭其中很小的检查项。

本文教如何建立合同和验收，不提供通用模型兼容表，也不声称运行过 ONNX Runtime、TensorRT、OpenVINO、DSP SDK、NPU SDK 或真实开发板。下文的标准库例子只验证自己的数据和比较规则。选定真实模型后，仍须完成模型质量评价、运行时集成和目标实测。

## 一  先定义算法与部署合同

先问算法解决什么问题，再问使用什么库。传统 FIR 滤波、PID 控制或几何优化可以直接实现为 C/C++ 模块；SLAM 往往还含标定、坐标系、时间同步、地图和状态估计。它们不因为输入是图像就自动变成神经网络部署。神经模型则还需要权重、图结构、算子和推理运行时。混合系统应为每一块分别保存合同。

例如把图像分类器接入采集链，接口不能只写“输入图片，输出类别”。至少写出：

| 合同项 | 必须可核对的内容 |
| --- | --- |
| 原始输入 | 来源、像素格式、通道顺序、行跨度、范围、采集序号与时间域 |
| 前处理 | 裁剪或补边、插值规则、RGB/BGR、缩放和均值、布局、计算精度 |
| 模型输入 | 名字、数量、shape、动态维范围、dtype、量化 scale/zero point |
| 模型输出 | 名字、shape、dtype；logits/概率/坐标的意义和坐标变换 |
| 后处理 | 阈值、排序和并列规则；检测时的 NMS 和类别映射版本 |
| 运行边界 | 最大帧大小、最多在途请求、过期策略、错误码、可取消阶段 |
| 质量与安全 | 评价数据、容差或任务指标、拒绝条件、低置信结果如何处理 |

前处理顺序是合同的一部分。例如先除以 255 再减均值，与先减以原始像素单位定义的均值再缩放，只有在参数对应时才等价。shape 相同也不能证明布局相同。检测输出的归一化坐标要按训练时的补边和缩放逆变换；信号算法则要保存采样率、窗口长度、单位与滤波状态的初始化。

合同中还应规定状态：每帧独立，还是相邻帧共享隐藏状态或历史窗口？重新连接、跳帧、重启和切换模型时谁重置状态？状态混用可能产生数值合理、语义错误的结果。

## 二  转换链路中的四道验收

建议把“源模型 → 交换格式 → 目标运行时产物”分成四道关卡，分别保存输入、命令、输出和版本。

1. 结构检查：读出输入输出、IR 和各 domain 的 opset，检查外部权重文件、动态 shape 和自定义算子。ONNX 的 IR、opset 和模型自己的版本是不同层；文件被检查器接受不代表目标后端实现了所有算子。[ONNX v1.16.1 的 IR](https://github.com/onnx/onnx/blob/595228d99e3977ac27cb79d5963adda262af99ad/docs/IR.md)与[版本规则](https://github.com/onnx/onnx/blob/595228d99e3977ac27cb79d5963adda262af99ad/docs/Versioning.md)用于核对这些层次。
2. 转换正确性：对同一组原始样本，用源框架参考执行和转换后 CPU 路径逐层或逐输出比较。先排查前处理、布局、shape 和后处理，再判断差异是否来自算子语义或数值精度。转换器没有报错不等于数值一致。
3. 优化正确性：FP16、INT8、图融合、裁剪和自定义算子分别引入变量。每次只改明确的一项或一组可解释设置，保存优化前后的产物及评价。
4. 目标验收：实际 CPU/DSP/GPU/NPU 路径重复质量评价，再测完整应用链的资源、时延和恢复。目标驱动、设备固件或运行时升级后重验关键样本。

对于无法转换的算法，可以保留一个清晰的 C/C++ 处理接口；不要为了一个格式强行改写整个系统。对于模型导出，必须查所选转换器和目标后端对应版本的算子、dtype、动态 shape 与插件限制。不要把不同 SDK 的文件格式或离线编译产物当可互换二进制。

## 三  目标与运行时按约束选择

| 目标类别 | 先核对什么 | 不可由别的环境推出什么 |
| --- | --- | --- |
| 小 MCU | Flash/RAM、算子内核、定点范围、栈和 scratch、周期预算、外设干扰 | PC 的内存分配方式、吞吐或通用 Linux 库支持 |
| ARM Linux CPU | AArch32/AArch64、ABI、指令集、libc、线程和缓存、包的构建配置 | 名称里有 ARM 就支持全部板卡或 NPU |
| DSP | 固定 SDK/编译器、向量与定点格式、对齐、共享内存、调用和完成协议 | DSP 与 Cortex-M 的 DSP 指令扩展具有同一运行时 |
| Linux 加速器 | 精确设备型号、驱动/固件、支持算子、设备内存、传输及同步 | 能加载图就全部在加速器执行，或 NPU 都使用相同 SDK |

ONNX 是表示模型的格式，ONNX Runtime 是执行模型的运行时，Execution Provider 是运行时接入后端的机制。TensorRT 和 OpenVINO 是有各自硬件、版本与构建限制的选项，不是每块 ARM 板都能使用的必选步骤。TensorRT v10.0.1 的[官方源码说明](https://github.com/NVIDIA/TensorRT/blob/d2f4ef789a9a6ffdf37b55c3f81b486225f6b380/README.md)也区分开源组件与单独提供的核心二进制；不要把仓库许可证扩大为全部 SDK 的许可结论。

MCU 的一个查证入口是 [CMSIS-NN v6.0.0](https://github.com/ARM-software/CMSIS-NN/blob/69823017d0973440e311dcf900b2162b17609fb0/README.md)。其优化路径、量化内核、编译器及算子范围需按该版本核对；它不是任意 ONNX 图的加载器。传统控制/DSP算法可以完全不使用模型交换格式。SLAM 还应单独验证标定与轨迹误差，不能用分类准确率替代。

本篇引用的是可复查的文档版本，不是“最新版本”或生产安装建议。实际选型应固定正在支持的 SDK 组合，并先查看该组合的支持矩阵、已知问题和更新说明。

## 四  量化要留下校准与评价证据

简化的仿射量化可写成 q = clamp(round(x / scale) + zero_point)，反量化为 x_hat = scale × (q − zero_point)。这里仍要明确整数范围、舍入规则、按张量或按通道量化，以及后端实际支持的表示。半精度与整数化也不是同一操作。

校准集帮助估计范围或分布，不自动证明任务质量。保存其样本清单或可复现生成规则、数据来源和获准用途、前处理版本、算法、样本数与参数。训练、校准和最终评价的角色要分开；避免用同一小组样本既调参数又宣称泛化。无需公开私人原始数据，公开可复现描述和获准样本即可。

对代表性条件分组评价：光照、噪声、输入尺度、少见类别、遮挡、饱和信号或传感器漂移。同时保存未量化基线、量化产物、任务指标和差异最大的样本。输入被 clipping 的比例与错误分布通常比“文件变小了”更有解释力。ONNX Runtime v1.18.1 的[校准工具源码](https://github.com/microsoft/onnxruntime/blob/387127404e6c1d84b3468c387d864877ed1c67fe/onnxruntime/python/tools/quantization/calibrate.py)可核对数据读取和不同校准方法；它不为你的数据选择自动提供质量保证。

不能统一规定“误差低于 1% 就合格”。分类、回归、检测、控制和安全相关决策需要不同的验收指标。先声明允许的质量损失与失败策略，再优化；不要看到结果后随意放宽阈值。

## 五  核对分区与回退

运行时可能将支持的子图交给一个后端，其余节点交给另一个后端，也可能因配置或缺少内核而直接失败。ONNX Runtime v1.18.1 的[图分区源码](https://github.com/microsoft/onnxruntime/blob/387127404e6c1d84b3468c387d864877ed1c67fe/onnxruntime/core/framework/graph_partitioner.cc)按 provider 的能力分配节点；不能仅因注册了 provider 就宣布全图加速。

验收时记录实际 provider 顺序、每段图所在设备、unsupported 节点、插入的拷贝与布局转换、启动日志和 profiling 证据。若产品要求完全使用某加速器，应检查并拒绝不符合要求的路径；若允许 CPU 回退，须重新检查回退后的质量、内存与 deadline。回退是明确的产品策略，不是默默发生的“总能运行”。

例如一段图在 NPU，一段后处理在 CPU，再返回 NPU；即使两段核函数各自很快，跨设备传输与同步仍可能主导端到端时间。小输入也可能被提交开销主导。这里的例子是待验证的设计假设，没有目标测量值。

## 六  输入输出缓冲与异步寿命

给每个请求分配序号和拥有者，把 buffer 的状态画成“可写 → 已提交 → 使用中 → 完成可读 → 消费结束可复用”。借用指针、拥有副本、引用计数和零拷贝是不同合同。零拷贝若延长相机 buffer 的占用，也会影响采集池容量。

- 提交成功只表示请求被接收或排入设备队列，不能立即覆盖输入。
- 超时表示调用者不再愿意等，不一定表示设备已停止访问。取消后仍须按运行时合同等待完成或确认取消，再回收资源。
- 输出视图的寿命通常受请求或存储对象约束。向 UI 或其他线程传递前，决定复制、移动或保留共享拥有权。
- 回调应携带请求序号和代次，丢弃旧代次结果；回调返回前不要销毁它仍引用的状态。

版本化 API 能帮助辨认具体边界：ONNX Runtime v1.18.1 的[用户内存 Tensor 接口](https://github.com/microsoft/onnxruntime/blob/387127404e6c1d84b3468c387d864877ed1c67fe/include/onnxruntime/core/session/onnxruntime_c_api.h)不会替调用者取得原始 buffer 的所有权。TensorRT v10.0.1 的 [enqueueV3 合同](https://github.com/NVIDIA/TensorRT/blob/d2f4ef789a9a6ffdf37b55c3f81b486225f6b380/include/NvInferRuntime.h)区分输入消费事件与输出的 stream 同步。OpenVINO 2024.3.0 的 [InferRequest](https://github.com/openvinotoolkit/openvino/blob/1e3b88e4e3f89774923e04e845428579f8ffa0fe/src/inference/include/openvino/runtime/infer_request.hpp)和[外部内存 Tensor](https://github.com/openvinotoolkit/openvino/blob/1e3b88e4e3f89774923e04e845428579f8ffa0fe/src/core/include/openvino/runtime/tensor.hpp)同样需要区分请求完成、共享对象寿命和调用者提供的存储。

线程、停止和资源回收机制见 [LX09](../70-linux-apps/lx09.md)、[LX17](../70-linux-apps/lx17.md)。本篇不再展开线程池实现。

## 七  内存与时间预算覆盖完整链路

内存清单应覆盖权重、常驻运行时、输入输出、前后处理、scratch/workspace、多个请求、采集和结果队列、线程栈及系统预留。先画寿命，判断哪些峰值会同时出现，再测量。不要把分配器高水位当当前活跃对象大小，也不要把进程 RSS 当全部设备内存；统一内存系统更要说明计数口径，避免重复相加。

算术例子：一个紧密排列的 224 × 224 × 3 float32 输入需要 602112 字节；三个互不共享的在途输入单这一项就需要 1806336 字节。uint8 同 shape 只需要 150528 字节，但转换时两个版本可能同时存在。此处没有包含权重、padding、workspace 或任何平台开销，不是模型总内存或板卡实测值。

完整时延从指定的采集/到达点开始，到业务消费者实际接受结果结束。记录采集、排队、前处理、上传、推理完成、下载、后处理和展示/动作各时间点；重叠阶段不能简单相加。设备 timestamp 与主机单调时钟不一定同源，必须记录转换或同步误差。推理核函数时间、请求响应时间和相机到显示时间分别报告。

帧率至少分成输入、接收、完成、有效结果与丢弃速率。批处理能增加吞吐，也可能增加排队时间。以每秒 30 帧输入、20 帧服务且持续不变的理想模型为例，容量 6 帧只吸收约 0.6 秒净积压；它不保证每帧的等待上界。真实服务长尾、同步和故障仍要测。预览可丢旧帧以保新鲜，存档或控制不能套用同一规则。背压与失败验收见 [DB05](../100-debugging/db05.md)。

目标测量要保存代表性数据集、分辨率、batch、请求并发、预热规则、样本数、运行长度、p50/p95/p99与最大观测时延、丢弃和错误数。不要只测一个可重复缓存的输入。分别覆盖冷启动、稳定负载、突发、长运行及资源竞争，并报告仪器/日志开销。

热和能耗须在实际设备测：保存环境温度、散热、功耗模式、频率、温度与降频时间线。运行长度应足以观察所声明的稳态或实际使用周期。能耗应说明测量边界与采样方法，例如板级供电能量积分除以有效结果数，并报告空闲基线和丢弃；CPU 占用率不能代替能耗。不把桌面推理 benchmark 换一个标签写成板级指标。

## 八  Golden 输出与版本清单

每个 golden 样本保存原始输入标识、前处理合同、参考输出、参考程序版本和可复现命令。浮点比较先核对 shape、dtype 和元素数，再处理 NaN/Inf 策略，最后使用预先声明的绝对及相对容差。一般元素级规则可写为 abs(actual − reference) ≤ atol + rtol × abs(reference)，但它不是所有任务的质量标准。接近零时只有相对误差会失真；只检查 top-1 相同也可能漏掉大幅 logit 漂移。

部署清单至少保存：算法/权重与外部文件哈希、导出器、IR/opset、自定义算子、量化和校准配置、runtime/provider、编译器/ABI、设备/驱动/固件、前后处理、标签与阈值、动态 shape 范围、测试数据标识和已知限制。模型、数据、代码和 SDK 的许可分别核对；[ONNX 的 Apache-2.0](https://github.com/onnx/onnx/blob/595228d99e3977ac27cb79d5963adda262af99ad/LICENSE)、[OpenVINO 的 Apache-2.0](https://github.com/openvinotoolkit/openvino/blob/1e3b88e4e3f89774923e04e845428579f8ffa0fe/LICENSE)或 [ONNX Runtime 的 MIT](https://github.com/microsoft/onnxruntime/blob/387127404e6c1d84b3468c387d864877ed1c67fe/LICENSE)并不能替模型权重和数据授予许可。[版本证据原则](../100-debugging/db06.md)仍适用。

## 九  一个可运行的主机合同模型

下例仅处理两像素 RGB 输入，把它转换为 NCHW float32，并检查一个原创仿射打分的参考结果。它不是训练好的识别模型，不导出 ONNX，不测推理库或目标性能。复制为 Python 文件直接运行即可。

```python
import math
import struct

def preprocess(raw):
    if len(raw) != 6:
        raise ValueError("expected two RGB uint8 pixels")
    owned = bytes(raw)  # original producer may reuse its bytearray
    nchw = [owned[p * 3 + c] / 255.0 for c in range(3) for p in range(2)]
    return struct.unpack("<6f", struct.pack("<6f", *nchw))

def compare(actual, reference, atol, rtol):
    if len(actual) != len(reference) or atol < 0 or rtol < 0:
        return False
    if not math.isfinite(atol) or not math.isfinite(rtol):
        return False
    return all(math.isfinite(a) and math.isfinite(b)
               and math.isfinite(a - b)
               and math.isfinite(atol + rtol * abs(b))
               and abs(a - b) <= atol + rtol * abs(b)
               for a, b in zip(actual, reference))

def score(nchw):
    if len(nchw) != 6:
        raise ValueError("expected shape [1, 3, 1, 2]")
    red, green, blue = [sum(nchw[c * 2:c * 2 + 2]) / 2 for c in range(3)]
    return (2 * red - green - blue,)

producer = bytearray([255, 0, 0, 0, 0, 255])
x = preprocess(producer)
producer[:] = bytes(6)
assert x == (1, 0, 0, 0, 0, 1)
assert compare(score(x), (0.5,), 1e-6, 1e-5)
assert not compare((float("nan"),), (0.5,), 1e-6, 1e-5)
assert not compare((), (0.5,), 1e-6, 1e-5)
print("host contract model passed")
```

比较中间计算若产生非有限误差或容差，本例保守拒绝；它不是针对任意数值范围的生产比较库。

本次核查在 Python 3.12.14 执行了此示例，并对长度拒绝、通道顺序、buffer 快照、非有限值、容差边界、简化量化 clipping 和预算算术补充主机测试。上述通过只覆盖这些原创模型。未执行模型训练、格式转换、量化工具、推理 provider、异步设备访问、温度、功耗或实板测量。

## 十  下一步练习与验收

选一个获准使用的真实算法或模型，先在 PC 上固定参考结果，再选择一块明确的目标。可以先做可复现的 FIR 信号链，也可以做一个有权重和评价数据的神经模型；任务目标不同，验收指标分别定义。

1. 给出输入输出合同与可拒绝输入，不省略前后处理。
2. 对真实模型写出源执行、转换、优化、目标的四栏结果，标明通过、失败或未运行。
3. 列出目标每个算子/模块的实际执行位置，检查回退策略。
4. 画两次并发请求的 buffer 寿命，注入超时、取消和旧结果到达。
5. 在实际目标记录内存峰值、端到端分位时延、有效帧率、队列峰值和异常终态；声明热/能耗是否实测。

答辩时能说明一次转换错误如何被 golden 样本发现、一次性能变化来自哪里，以及未完成哪一层。没有目标设备结果时，成果应称“主机合同原型”或“转换验证”，不能称板级部署完成。

正文、接口合同、例子与练习为原创教学内容。公开官方来源用于核对机制和版本，不表示第三方为本手册背书。
