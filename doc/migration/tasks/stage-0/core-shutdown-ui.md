# 模块迁移文档：shutdown-ui

## 1. 路径
- 原项目路径：`cutefish/code/core/shutdown-ui`
- 迁移后路径：`cutefish/wayland-code/core/shutdown-ui`

## 2. 依赖变化
- 迁移前后：Qt Core/Widgets/Quick/QuickControls2/DBus；无 X11 依赖。

## 3. 架构变化
- stage-0 保留旧关机对话框作为包边界；stage-3 将删除并合并到 `cutefish-shell/ShutdownShell`。
- 已删除 QML 中 `Qt.X11BypassWindowManagerHint`。

## 4. 对外接口变化
- 无。

## 5. 功能差异与首版裁剪项
- 首版裁剪：关机 UI 最终不保留独立进程。

## 6. 测试与验收状态
- 编译通过；未做关机动画验收。
