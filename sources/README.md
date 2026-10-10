# 公开参考资料与核查范围

公开技术文档用于机制核对与进一步阅读；核查范围和适用版本分别记录，不表示相关作者背书。

- [课程官方资料目录](core-catalog.json)
- [官方来源与适用边界](official-catalog.json)
- [补充官方条款核查](primary-verification.json)
- [公开面经便利样本](interview-sample.json)：只用于观察问法，不代表统一考频。

完整约束以对应官方文档为准。公开链接可随维护更新，实际工程应固定目标版本、工具链和配置。

- [新增公开自报页面目录](../docs/120-interviews/source-catalog.md)、[结构化来源](interview-supplement.json)：17份完整页面，试读和无日期GitHub日志另列。
- [七个原创故障场景](../docs/120-interviews/README.md)：问题主题与官方技术答案分开，练习均尚未执行。

## FreeRTOS源码扩展

- [7篇原创扩展与阅读顺序](../docs/50-rtos/source-study-manifest.json)、[源码导读](../docs/50-rtos/source-reading.md)：章节引用公开固定版本源码，用于机制核对；手推与设计练习尚未运行。
- [现代V11.1.0固定提交](https://github.com/FreeRTOS/FreeRTOS-Kernel/tree/dbf70559b27d39c1fdb68dfb9a32140b6a6777a0)，单核GCC ARM_CM4F，MIT；[历史V9.0.0固定提交](https://github.com/FreeRTOS/FreeRTOS/tree/165c24cba94e3d3f386c3b77d3bb0f4a524d8511)仅用于co-routines选读，GPLv2加FreeRTOS exception。
- 引用范围是各章实际讨论的文件与分支，不声称读完所有版本、所有配置或全部官方文档；没有转载上游实现与注释。
