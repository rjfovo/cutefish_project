# 阶段 1：核心合成器与 Shell 完整闭环

> 所属主计划：`../wayland-migration-master-plan.md`
> 架构依据：`../cutefish-dedicated-compositor-architecture.md`
> 目标：`cutefish-compositor-core` 完成显示、输入、协议、窗口管理和最终合成；
> `cutefish-shell` 作为可信 Qt Quick Wayland 客户端承载全部用户 UI。

## 1. 需要修改的项目

| 项目/路径 | 修改内容 |
|---|---|
| 新增 `cutefish/code/cutefish-compositor-core` | KMS backend、libinput、logind、libwayland-server、WM、最终合成、ShellManager、portal、auth |
| 新增 `cutefish/code/cutefish-shell` | Qt Quick Wayland 客户端：Boot/Login/Session/Lock/Shutdown、Desktop/Dock/StatusBar/Launcher |
| `script/build_code.sh`、`.gitmodules`、`script/download_codes.sh` | 新增 core/shell 子模块和构建条目 |
| `script/create_iso/build_live_filesystem.sh`、`build_install_filesystem.sh` | 安装 core/shell 新包 |
| `build_iso/package/cutefish` | 覆盖 core/shell deb 的构建、复制和仓库重建流程 |
| `core/session` | 删除 X11/KWin 启动；只负责应用环境、用户服务、D-Bus |
| `desktop` | QML 迁入 cutefish-shell；删除 KX11Extras/桌面窗口类型代码 |
| `desktop-background` | 并入 cutefish-shell/desktop 后删除 |
| `dock` | QML 迁入 cutefish-shell；任务栏数据来自 `cutefish_core_v1` |
| `statusbar` | QML 迁入 `cutefish-shell`；活动窗口/通知 role 走 `bind_shell_surface`，AppMenu 走用户会话 D-Bus Registrar |
| `launcher` | QML 迁入 cutefish-shell；激活走 core |
| `filemanager` | 桌面图标层迁入 shell；主程序保持普通 Wayland 应用 |
| `fishui` | 删除 X11 windowhelper/windowblur；提供统一 `Cutefish.Window/TitleBar` CSD 组件 |
| `qt-plugins` | 删除 x11integration；通过用户会话 D-Bus AppMenu Registrar 注册 service/path |
| `settings` | 删除 Xcursor/Xft X server 查询；屏幕设置页走 `com.cutefish.Display` |
| `screenshot` | 删除 X11 grabWindow；改走 core portal |
| `screenlocker` | 旧进程删除；core 持有锁定状态，shell 显示 LockShell |
| `core/chotkeys` | 删除 XGrabKey；快捷键在 core 输入链 |
| `core/gmenuproxy` | 删除 xcb；删除进程或重写为普通用户级 D-Bus 服务，不访问 `cutefish_core_v1` |
| `core/settings-daemon` | 删除 X11 鼠标/触控板后端；写 libinput/core 配置 |
| `core/powerman` | 删除 X DPMS；DPMS/电源走 core + logind；明确 KF6IdleTime 去留 |
| `core/clipboard` | 删除旧客户端进程；剪贴板管理移入 core |
| `core/notificationd` | 仅保留 D-Bus 数据服务；通知 UI 移入 `cutefish-shell` |
| `libcutefish/screen` | 删除 KScreen，改为 `com.cutefish.Display` D-Bus 客户端 |
| `settings` | 光标主题解析改为 libwayland-cursor 或 core 预览接口 |
| `core/xembed-sni-proxy` | 删除，系统托盘只支持 SNI |
| `core/cupdatecursor` | 删除，光标由 core 管理 |
| `fantascene-dynamic-wallpaper` | 删除 X11 版本；视频壁纸迁入 Shell 或后续 compositor 能力 |
| `kwin-plugins` | 删除；装饰、阴影、圆角、Alt-Tab 由 core/shell 实现 |
| `yoyo-fantacy` | 从 ISO 构建脚本删除；X11 视频壁纸包不再安装 |

## 2. 依赖

### 2.1 compositor core

```text
Qt6::Core / Gui / DBus        # 不依赖 Qt Quick
libwayland-server
libdrm / libgbm
libinput / libxkbcommon / libudev
PAM / systemd-logind
Mesa / EGL
wayland-protocols
vendored wlr-protocols（zwlr_data_control_v1 XML）
```

### 2.2 Shell 和应用

```text
Qt6::Core / Gui / Quick / QuickControls2 / DBus
qt6-wayland（客户端 QPA）
qml6-module-qtquick
libwayland-cursor-dev（settings 光标主题解析）
```

阶段 1 后续实施 portal/PipeWire 录屏时补充：

```text
xdg-desktop-portal 协议/开发依赖
libpipewire-0.3-dev
```

### 2.3 禁止

```text
QtWaylandCompositor API 链接与调用
eglfs_kms
wlroots
KWin / KDE Plasma / KScreen / KWindowSystem
XWayland
```

## 3. 需要解决的问题

1. 核心必须实现最小 xdg-shell，包括 xdg_popup/positioner 的正确约束求解。
2. 核心必须实现最终合成和窗口级模糊、圆角、阴影、动画。
3. Shell 是普通 Wayland 客户端，不能直接读取应用窗口纹理。
4. 窗口缩略图、Dock 预览、Alt-Tab 由 core 导出，锁屏/隐私下禁止导出。
5. 剪贴板管理在 core，关闭应用后保留；主选择和中键粘贴可用。
6. DnD 由 core 管理 drag surface、drop/cancel，Shell 只做可视反馈。
7. 通知窗口位置由 core 私有窗口规则决定。
8. 全局菜单地址绑定不再走 xcb。
9. 输出管理：settings 通过 `com.cutefish.Display` D-Bus 访问，不能访问 `cutefish_core_v1`。

## 4. 功能变化

| 功能 | 变化 |
|---|---|
| 显示服务 | X11/KWin → `cutefish-compositor-core` |
| 桌面/Dock/状态栏/启动器 | 独立进程 → `cutefish-shell` 内 QML |
| 窗口管理 | KWin → core WM |
| 窗口特效 | KWin effect → core 最终合成特效；提供降级开关 |
| 窗口装饰 | KWin decoration → FishUI CSD + core move/resize |
| 快捷键 | XGrabKey → core 输入链 |
| 截图 | X11 grabWindow → core 最终帧 + portal |
| 剪贴板 | 普通客户端 → core data-control |
| 全局菜单 | xcb → 用户会话 D-Bus Registrar；Shell 经 core 获得 focused app_id 后查询 D-Bus |
| 视频壁纸 | X11 EWMH → Shell 原生视频层 |
| 托盘 | XEmbed+SNI → 仅 SNI |

## 5. 可能遇到的问题

| 问题 | 应对 |
|---|---|
| xdg_popup/positioner 定位复杂 | 加入 Spike 8，重点验证 Qt Quick 菜单/ComboBox/ToolTip |
| 窗口纹理导出给 Shell | `cutefish_core_v1` 版本化缩略图接口；锁屏自动禁用 |
| 核心合成特效性能 | 先做固定参数特效，不做通用 effect 框架 |
| 剪贴板/DnD 协议缺失 | 使用 wayland-protocols 和 vendored wlr XML，自行实现 |
| 输出管理普通应用通路 | session Shell 注册 `com.cutefish.Display`，内部经 `cutefish_core_v1` 调 core |
| 私有协议隔离 | 双 socket：Shell 专用 socket 注册 `cutefish_core_v1`，普通应用 socket 不注册 |
| 非 Shell 用户服务与 core 的 IPC/D-Bus 边界 | powerman、settings-daemon、光标、portal、PipeWire 全部走“用户会话服务/helper → Shell 或受控 IPC → core” |
| 输入法 | P0：实现 text-input-v3 + `zwp_input_method_v2`，支持 fcitx5/ibus |
| 分数缩放 | 首版整数缩放，fractional-scale 不进入阶段 1 |
| Shell 崩溃 | core 重启 Shell，不重启显示服务 |

## 6. 验收标准

- `cutefish-compositor-core` 和 `cutefish-shell` 完整闭环；
- 应用窗口显示、移动、缩放、最小化、最大化、关闭正常；
- 桌面、Dock、状态栏、启动器功能与 X11 基线等价；
- 窗口模糊、圆角、阴影、Alt-Tab、Dock 预览正常；
- 截图、录屏、剪贴板、主选择、DnD、通知正常；
- 全局快捷键、输入设备、DPMS、锁屏、注销/重启正常；
- 系统无 Xorg、XWayland、KWin 进程；
- 对应 X11/KWin 代码已删除。

## 7. 交付物

1. `cutefish-compositor-core` 和 `cutefish-shell`。
2. `cutefish_core_v1` 协议实现。
3. 功能等价矩阵验收报告。
4. X11/KWin 代码删除记录。
5. 虚拟后端和真机测试报告。
