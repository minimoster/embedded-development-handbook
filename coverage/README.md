# 验证与覆盖

## 自动检查入口

```bash
python3 scripts/update_index.py
python3 scripts/validate.py --strict
python3 scripts/validate_public_metadata.py
python3 tests/run_host_tests.py
python3 tests/run_host_tests.py --sanitize --disable-leak-detection
node site/build.mjs
node site/build.mjs --check
node site/test.mjs
```

- core-question-manifest.json：原创核心题唯一清单，不包含算法题。
- [validation-summary.json](validation-summary.json)：最后验证结果与环境、限制；只在实际执行后更新。
- Hot100题解/源码/逐题测试使用算法自己的manifest与脚本，官方100和额外公开练习独立计数。

内容检查要求每个核心题具有七个结构区、足够正文、已登记官方来源；本地链接/锚点正确，公开内容不包含内部文件ID/下载token。静态构建hash确保源文与阅读产物一致。

主机测试不是板测；静态UI检查不是实际浏览器、触屏或辅助技术审核。LeakSanitizer在本容器ptrace下不能工作，禁用后只报告ASan/UBSan结果。无硬件可用时所有硬件结果写“未运行”。

本次重新执行了九组已有主机测试，以及全部101个已有算法入口的normal、ASan/UBSan和checked-iterator回归，另有17题独立有界穷举。已有核心、算法、测试与场景正文保持不变。LeakSanitizer默认运行再次被ptrace限制阻塞；明确关闭泄漏检测后ASan/UBSan通过。


## FreeRTOS源码扩展的验证边界

7篇原创扩展独立计数，120核心主题、RT01–RT10、官方Hot100的100题、1道额外算法题和7张已有故障场景保留各自清单。现代六篇固定V11.1.0、单核GCC ARM_CM4F；V9.0.0历史co-routines为P2选读，许可边界分别说明。章文SHA256、来源版本、Markdown围栏、本地链接、导航分类、全文检索与生成产物一致性已检查。阅读器现在包含255篇文档和231个离线资源。

这些章文的主机模型、内核、模拟器和板级练习未编译或运行；上述已有主机/算法回归不提供FreeRTOS调度、实时性或硬件证据。实际浏览器与部署检查按validation-summary.json分别记录，不把静态/DOM模拟称为实机手机、触屏或辅助技术测试。

本次实际Chrome采用本地HTTP预览，在1180×757 CSS桌面和385×621 CSS窄桌面（150%缩放）检查7篇新章。章名、渲染与源码视图SHA256一致；分类、检索、基础路线链接、RT10到7篇扩展的下一篇顺序、重复目录/历史导航、窄屏菜单与局部横向滚动通过。7篇窄屏均无页面级横向溢出。这是有界桌面浏览器检查，没有完成全部255篇实浏览器遍历、实机触屏手机、辅助技术或完整网络/错误审计。
