# 模块迁移文档：powerman

## 1. 路径
- 原项目路径：`cutefish/code/core/powerman`（只读审计参考）
- 迁移后路径：`cutefish/wayland-code/core/powerman`

## 2. 依赖变化
- 迁移前：Qt Core/Gui/Widgets/Quick/DBus、KF6IdleTime、X11/XCB/DPMS。
- 迁移后：Qt Core/Gui/Widgets/Quick/DBus、KF6IdleTime；移除 X11/XCB/DPMS。

## 3. 架构变化
- `dimdisplayaction.cpp` 删除 X11 DPMS/XScreenSaver 路径。
- 亮度 idle 策略继续调用 `com.cutefish.Settings.Brightness`。
- 锁屏请求改为 `loginctl lock-session`；最终由 core 锁屏状态机替代。
- 睡眠/恢复由 logind 负责。

## 4. 对外接口变化
- D-Bus `com.cutefish.PowerManager` 和 `/PowerManager` 保持。
- ScreenSaver 接口保持。
- 不再写 X DPMS 超时。

## 5. 功能差异与首版裁剪项
- 首版裁剪：无直接 DPMS 属性操作；DPMS 由 core DisplayBackend 负责。
- KF6IdleTime 保留为阶段白名单依赖。

## 6. 测试与验收状态
- 编译通过；dbus-run-session + offscreen 冒烟通过。
- 未验收：真实 idle/锁屏/睡眠恢复联调。

## 7. 注释要求
- `dimdisplayaction.cpp` 文件头说明 Wayland-only 亮度/锁屏策略边界。
