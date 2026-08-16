# 模块迁移文档：notificationd

## 1. 路径
- 原项目路径：`cutefish/code/core/notificationd`（只读审计参考）
- 迁移后路径：`cutefish/wayland-code/core/notificationd`

## 2. 依赖变化
- 迁移前：Qt Core/DBus/Quick/Widgets、KF6WindowSystem、FishUI；包含 QQuickView 通知弹窗。
- 迁移后：Qt Core/DBus；移除 KF6WindowSystem、FishUI、Quick/Widgets UI。

## 3. 架构变化
- 从“数据服务 + 本地 QML 弹窗”改为纯数据服务。
- 通知弹窗、定位、过期视觉由 `cutefish-shell` 通过 `bind_shell_surface` 通知 role 承担。
- `showWindow` D-Bus 方法保留兼容，但只记录请求。

## 4. 对外接口变化
- `org.freedesktop.Notifications` 接口保持。
- `com.cutefish.Notification` 的 showWindow/setDoNotDisturb/doNotDisturb 保持。
- 移除 QML 类型注册和窗口类。

## 5. 功能差异与首版裁剪项
- 首版裁剪：通知弹窗 UI 未随数据服务运行；待 Shell 通知 surface 接入。
- Do Not Disturb 与历史模型保留。

## 6. 测试与验收状态
- 编译通过；dbus-run-session 冒烟通过。
- 未验收：Shell 通知 UI 联调。

## 7. 注释要求
- `application.cpp` 文件头说明数据服务边界，避免误认为仍管理弹窗。
