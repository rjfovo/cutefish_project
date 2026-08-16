# stage-1 任务：Shell QML 窗口/输出模型对接

- 状态：进行中

## 模块迁移元数据

- 原项目路径：无（新增 QML 模型）
- 迁移后路径：`cutefish/wayland-code/cutefish-shell/{outputlistmodel,windowlistmodel}`
- 迁移前依赖：不适用
- 迁移后依赖：Qt Core/QML
- 架构变化：QML 使用 QAbstractListModel 消费 core v2 输出/窗口事件
- 对外接口变化：OutputModel/WindowModel/CoreClient 注入 QML 上下文
- 功能差异/裁剪：桌面图标/启动器应用模型仍为占位
- 测试与验收：offscreen QML 测试通过

## 实际修改

- `OutputListModel`：name/width/height/scale/transform/connected roles。
- `WindowListModel`：windowId/appId/title/state/activated roles。
- `Desktop/Dock/StatusBar/Launcher` 替换纯占位文字，开始消费模型。
- Dock 点击调用 `CoreClient.requestActivate(appId)`。
