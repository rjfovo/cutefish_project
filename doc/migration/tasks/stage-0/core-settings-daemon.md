# 模块迁移文档：settings-daemon

## 1. 路径
- 原项目路径：`cutefish/code/core/settings-daemon`（只读审计参考）
- 迁移后路径：`cutefish/wayland-code/core/settings-daemon`

## 2. 依赖变化
- 迁移前：Qt Core/Gui/Widgets/DBus/Xml、X11/XCB、xorg-libinput、xorg-server、xorg-synaptics；鼠标/触控板走 XInput2/Synaptics/XRecord。
- 迁移后：Qt Core/Gui/Widgets/DBus/Xml；移除全部 X11/XCB/xorg 依赖；鼠标/触控板改为 QSettings 配置数据服务，实际 libinput 配置由 compositor core 消费。

## 3. 架构变化
- Theme/Brightness/Battery/Language/Dock/DefaultApplications 保留用户会话 D-Bus 数据服务。
- Mouse/Touchpad 从 XInput2/Synaptics 后端改为 Wayland-only 配置持久层。
- `applyXResources`/`applyCursor` 不再执行 xrdb/cupdatecursor，只持久化配置。
- 移除 sddm-helper DPI 调用和 KWin effect 初始化注释/代码。

## 4. 对外接口变化
- D-Bus 服务名 `com.cutefish.Settings` 保持。
- Theme/Brightness/PrimaryBattery/Language/Dock/Mouse/Touchpad 对象路径保持。
- Mouse/Touchpad 属性语义保持，但写入目标从 X 设备属性改为 Cutefish 配置文件。

## 5. 功能差异与首版裁剪项
- 首版裁剪：不直接打开 libinput 设备；设备可用性由 core 后续提供。
- 触摸板 available 暂时返回 true，待 stage-1 `com.cutefish.Input` 接入真实状态。

## 6. 测试与验收状态
- 编译通过；dbus-run-session + offscreen 冒烟通过。
- 未验收：真实 libinput 配置端到端、显示/输入设置页联调。

## 7. 注释要求
- `mouse/mousemanager.*`、`touchpad/touchpadmanager.*` 已注明 Wayland-only 配置职责。
- `theme/thememanager.cpp` 已删除 X 资源命令和误导性注释。
