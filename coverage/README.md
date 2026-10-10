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

本次实际浏览器/手机模拟渲染检查因运行环境阻塞未完成；部署后仍需核验在线页面。已经通过的是静态渲染/搜索/本地路由检查，不能写成手机或真实浏览器测试通过。
