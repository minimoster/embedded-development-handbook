# 可离线的静态阅读器

下载整个仓库，用现代浏览器打开site/index.html；无需fetch、账号、后端、CDN、字体或联网。也可在仓库根运行：

```bash
python3 -m http.server 8000 --directory site
node site/build.mjs
node site/build.mjs --check
node site/test.mjs
```

章内目录支持直接定位、重复点击、浏览器前进和后退；所有学习入口以课程讲义为目标。

支持中文全文搜索、专题、课程优先级、算法难度、章内目录、解答折叠、明暗主题、字号/行距、收藏/已读和继续阅读。阅读记录仅保存在当前浏览器，不上传、不跨设备同步。代码和关联测试随离线文件提供；外部来源仅在主动点击时联网。

手机导航支持Esc关闭与焦点约束；长代码/表格在局部滚动，不应使整页横向溢出。静态检查不能代替真实浏览器/触屏/辅助技术验证。

assets/data.js与assets/repo是生成产物，不在那里修正文；文档与manifest是源。Hash路由支持静态托管和file://打开。公开仓库不等于在线部署已成功，线上URL需发布后单独核验。
