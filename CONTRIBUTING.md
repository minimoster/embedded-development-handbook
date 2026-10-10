# 如何改进这本手册

1. 指出题号、错误句、适用平台/语言版本，并给官方证据或可复现反例。
2. 保留直接回答→机制→例子→误区→追问→验收→来源结构；优先修正因果和边界，不用堆名词增加数量。
3. 修改docs原文或examples/tests源码；不要只修改site/assets/data.js生成文件。
4. 更新core-question-manifest或相应算法manifest，运行内容/链接/构建与测试。
5. 不上传未授权第三方正文、二维码、联系方式、下载token、密钥、真实设备/个人敏感日志。外部资料默认只链接；许可证与实际阅读状态保留。

```bash
python3 scripts/update_index.py
python3 scripts/validate.py --strict
python3 scripts/validate_public_metadata.py
python3 tests/run_host_tests.py
node site/build.mjs
node site/build.mjs --check
node site/test.mjs
```

若目标硬件或工具不可用，写“未运行”及需要的验证条件。不要将测试失败或已知限制删掉来制造通过。

FreeRTOS源码扩展使用docs/50-rtos/source-study-manifest.json维护独立分类、阅读顺序和章文哈希，不放入核心题manifest。修改这些章文时同步更新其SHA256，再运行scripts/validate_source_study.py及全部阅读器检查。现代与历史版本/许可分别核对；实际内核与板级实验未执行时保持该边界。
