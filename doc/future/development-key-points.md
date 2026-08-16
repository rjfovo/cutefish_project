# CutefishOS Wayland 研发重点要点清单

> 文档目的：防止研发过程中遗忘关键约束、改造点和验收条件。
> 适用架构：`cutefish-compositor-core` + `cutefish-shell`。
> 总原则：**专用、简洁、稳定、可替换**。

---

## 0. 最高优先级约束

### 0.0 四原则是最高准则

**专用、简洁、稳定、可替换** 是整个 CutefishOS 桌面开发的最高准则和长期指导方针。

### 0.1 迁移衍生决策：无过渡期

“无过渡期”只针对本次 X11/KWin → Wayland/Qt 迁移，是四原则下的衍生决策：

- 产品不发布新旧并存的迁移过渡版本；
- X11/KWin/SDDM/Calamares 不进入产品运行时过渡；
- 旧实现只存在于 git 历史；
- 每个模块直接切换到最终实现，验收通过后才进入产品版本。

以下约束违反任意一条，代码不允许合入主线：

1. 不引入 KWin、KDE Plasma、KScreen、KWindowSystem。
2. 不链接、不调用 QtWaylandCompositor API；不引入 eglfs_kms、wlroots。
   原厂 Debian `qt6-wayland` 客户端插件会依赖 `libqt6waylandcompositor6`，
   该库文件可能存在于镜像中，但不得被 Cutefish 链接或调用。
3. 不引入 XWayland，不保留 X11 session。
4. 最终桌面运行时不运行 Xorg、XWayland、SDDM、KWin、Plymouth；
   Plymouth 只允许作为 initramfs splash 运行，rootfs 后必须退出。
5. 核心从开机到关机常驻，登录/注销不得重启显示服务。
6. Shell 不得持有 DRM fd、libinput fd、logind 控制权。
7. 应用不得访问 `cutefish_core_v1` 私有协议。
   实现必须是双 socket：Shell 专用 socket 注册私有协议，普通应用 socket 不注册。
8. 输入法为 P0，必须实现 `zwp_input_method_v2`，支持 fcitx5/ibus。
9. 所有可替换点必须走接口，不允许业务代码直接调 libdrm/libinput/logind。
10. 协议和接口必须版本化。
11. 新增进程、协议、依赖前，必须先证明产品必需。

---

## 1. 架构改造要点

### 2.1 目标进程模型

```text
cutefish-compositor-core
  ├── 常驻、特权
  ├── KMS/GBM、libinput、logind、Wayland server、WM、最终合成
  ├── 截图/录屏授权、锁屏状态机
  └── 管理 cutefish-shell

cutefish-shell
  ├── 可信 Qt Quick 客户端
  ├── Boot/Login/Lock/Session/Shutdown UI
  ├── Desktop/Dock/StatusBar/Launcher
  └── 崩溃后可被 core 重启

Cutefish 应用
  └── 普通 xdg-shell 客户端
```

### 2.2 必须新建的项目

```text
cutefish/code/cutefish-compositor-core
cutefish/code/cutefish-shell
```

### 2.3 必须重构的现有项目

| 项目 | 关键改造 |
|---|---|
| core/session | 删除 X11/KWin 启动；只负责应用环境、用户服务、D-Bus |
| desktop | QML 迁入 cutefish-shell，删除 X11 窗口类型代码 |
| desktop-background | 合并入 shell/desktop 后删除 |
| dock | QML 迁入 cutefish-shell；任务栏数据来自 cutefish_core_v1 |
| statusbar | QML 迁入 cutefish-shell；AppMenu 走用户会话 D-Bus Registrar |
| launcher | QML 迁入 cutefish-shell；使用 xdg-activation/私有协议 |
| filemanager | 桌面图标层迁入 shell；主程序保持 Wayland 应用 |
| fishui | 删除 X11 windowhelper/windowblur；特效由 shell/core 分工 |
| qt-plugins | 删除 x11integration；通过用户会话 D-Bus AppMenu Registrar 注册 service/path |
| settings | 删除 Xcursor/Xft X server 查询；屏幕页接新输出管理 |
| screenshot | 截图走 core portal，不再 grabWindow |
| screenlocker | 旧进程删除；锁定状态在 core，UI 在 shell |
| core/chotkeys | 删除 XGrabKey；快捷键在 core 输入链 |
| core/gmenuproxy | 删除 xcb；删除进程或重写为普通用户级 D-Bus 服务，不访问 cutefish_core_v1 |
| core/settings-daemon | 删除 X11 鼠标/触控板后端；配置走 libinput/core |
| core/powerman | 删除 X DPMS；DPMS/电源走 core + logind |
| core/clipboard | 删除旧客户端进程；剪贴板管理移入 core |
| core/notificationd | 仅保留 D-Bus 数据服务；通知 UI 移入 cutefish-shell |
| libcutefish/screen | 删除 KScreen，改为 `com.cutefish.Display` D-Bus 客户端 |
| settings | 光标主题解析改为 libwayland-cursor 或 core 预览接口 |
| core/xembed-sni-proxy | 删除；系统托盘只支持 SNI |
| 窗口装饰 | FishUI 提供统一 CSD 标题栏；core 实现 move/resize 交互和边缘光标；GTK 应用白名单 |
| core/cupdatecursor | 删除；光标由 core 管理 |
| fantascene-dynamic-wallpaper | 删除 X11 版本；视频壁纸迁入 shell/desktop |
| kwin-plugins | 删除；窗口装饰改为 FishUI CSD；阴影/圆角/Alt-Tab 由 core/shell 实现 |
| yoyo-fantacy / fantascene-dynamic-wallpaper | X11 视频壁纸；ISO 当前仍安装 yoyo-fantacy，依赖 libx11/libxcb-ewmh，必须从构建脚本删除或重写为原生视频壁纸 |
| core/shutdown-ui | 删除或合并到 cutefish-shell 的 ShutdownShell |
| core/sddm-helper | 删除；同时移除 settings-daemon/theme 中的 `pkexec cutefish-sddm-helper --dpi` 调用点 |
| core/powerman | 删除 KF6IdleTime 依赖；idle 状态由 core 输入链提供，或保留 KF6IdleTime 并列入应用级 KF6 白名单 |
| sddm-theme | 删除；登录 UI 在 cutefish-shell |
| plymouth-theme | 缩小为 initramfs splash；最终由 BootShell 覆盖 rootfs 后阶段 |
| calamares | 直接从产品删除；唯一安装器为 `cutefish-installer` |
| calamares-settings | 直接从产品删除；不写入 SDDM/Plymouth/X11 session |

---

## 2. 核心合成器开发要点

### 3.1 backend

```text
DisplayBackend
  ├── KmsBackend：DRM/KMS、GBM、atomic modeset、page flip、DPMS、光标
  └── VirtualBackend：CI/调试
```

必须实现：

- 输出枚举、连接/断开事件；
- 模式设置和当前模式；
- buffer 创建和提交；
- 睡眠恢复；
- 核心崩溃后 systemd 重启并重新初始化后端。

### 3.2 input

```text
LibinputBackend
```

必须实现：

- 键盘、指针、触摸、触控板；
- xkbcommon 键盘布局；
- 设备热插拔；
- 输入焦点由 core 决定；
- 全局快捷键在应用分发前处理；
- 锁屏时普通客户端不可接收输入。

### 3.3 session

```text
LogindSession
```

必须实现：

- TakeDevice / ReleaseDevice；
- PauseDevice / ResumeDevice；
- VT 切换；
- 登录前后只切换 Shell，不重启 core；
- 用户会话由 logind/systemd --user 管理。

### 3.4 wayland server

必须实现的最小协议：

```text
wl_compositor
wl_subcompositor
wl_shm
wl_seat
wl_output
zxdg_output_manager_v1
wl_data_device_manager
wp_viewporter
xdg_wm_base
xdg_surface
xdg_toplevel
xdg_popup
wp_primary_selection_v1
zwlr_data_control_v1
zwp_linux_dmabuf_v1
zwp_text_input_manager_v3
zwp_input_method_v2
xdg_activation_v1
cutefish_core_v1
```

协议 XML 来源：

- `wayland-protocols` 提供 xdg-shell、linux-dmabuf、primary-selection、text-input、xdg-activation 等；
- `zwlr_data_control_v1` 属于 wlr-protocols，Debian 13 没有 wlr-protocols 包，必须 vendored XML。

不实现：

```text
XWayland
wlr-layer-shell
plasma-window-management
KWayland AppMenu
```

### 3.5 WM 策略

- 窗口创建、关闭、焦点、z-order；
- 最大化、最小化、全屏；
- Alt-Tab、任务栏窗口模型；
- 禁止应用自行定位、抢占焦点；
- 锁屏状态优先于所有窗口操作。

### 3.6 最终合成

- 应用窗口 buffer 统一通过 dmabuf/shm 进入核心；
- 核心生成最终帧并 page flip；
- Shell surface 由核心合成；
- 截图读取核心最终帧。

---

## 3. Shell 开发要点

### 4.1 必须实现的界面

```text
BootShell
LoginShell
SessionShell
LockShell
ShutdownShell
Desktop
Dock
StatusBar
Launcher
```

### 4.2 必须遵守的边界

- 不打开 DRM；
- 不处理 libinput 设备；
- 不执行 PAM；
- 不能截图其他应用；
- 不能注入全局输入；
- 不能覆盖锁屏；
- 通过 `cutefish_core_v1` 获取窗口模型和输出信息。

### 4.3 崩溃恢复

- Shell 崩溃后，core 保持当前显示状态；
- core 按当前生命周期重启 Shell；
- Boot/Login/Session/Lock 状态不能因 Shell 重启而丢失；
- Shell 重启必须重新协商 `cutefish_core_v1` 版本。

---

## 4. 私有协议开发要点

`cutefish_core_v1` 必须覆盖：

```text
输出列表和输出变化
窗口列表和窗口属性变化
活动窗口和焦点变化
激活/关闭/最小化/最大化请求
Dock/顶栏几何占位
启动器显示/隐藏
通知锚点和窗口规则
锁屏状态
截图/录屏请求与授权
bind_shell_surface(wl_surface, role, output, z_order)
surface_role_changed
Shell 就绪、心跳、重启
```

协议规则：

- XML 版本化；
- Shell 启动时协商版本；
- 应用永远不能 bind 该协议；
- 实现必须使用双 socket：
  - Shell 专用 socket：注册 `cutefish_core_v1`；
  - 普通应用 socket：不注册 `cutefish_core_v1`；
- 协议变更必须兼容或显式升级，旧版本只保留一个兼容周期。

---

## 5. 剪贴板、DnD、通知和 AppMenu 要点

### 6.1 剪贴板

- 剪贴板管理在 core，不能继续依赖普通客户端；
- 实现 `wl_data_device_manager`；
- 实现 `zwlr_data_control_v1`；
- 关闭应用后剪贴板必须保留；
- core 必须缓存当前 selection：文本、图片、文件列表 URI；
- 主选择缓存独立；
- 设置大小上限和清理策略，注销时清空。

### 6.2 主选择

- 实现 `wp_primary_selection_v1`；
- 中键粘贴必须可用。

### 6.3 DnD

- core 负责 drag surface、拖放图标、drop/cancel；
- Shell 负责可视反馈；
- 跨应用文件拖放必须验证。

### 6.4 通知

- notificationd 只负责数据和 D-Bus；
- Shell 创建通知 surface，并通过 `bind_shell_surface` 声明通知 role；
- core 根据 role 负责定位和层级；
- 通知点击、过期、跳转必须验证。

### 6.5 AppMenu

采用三段式，不允许普通应用访问 `cutefish_core_v1`：

```text
普通 Qt 应用：
  qt-plugins 通过用户会话 D-Bus AppMenu Registrar 注册 service/path

core：
  仅向 cutefish-shell 提供 focused app_id

cutefish-shell：
  根据 focused app_id 查询 D-Bus Registrar 并渲染菜单
```

- DBusMenu 数据面保留；
- 窗口地址绑定不再使用 xcb；
- `gmenuproxy` 删除，或作为普通用户级 D-Bus 服务重写；
- `gmenuproxy` 不得成为 `cutefish_core_v1` 的例外客户端。

---

## 6. 输出管理、屏幕设置和 KScreen 替换要点

- 删除 KF6::Screen/KScreen 在 libcutefish/screen 的依赖；
- core 通过 `cutefish_core_v1` 向 Shell 暴露输出、模式、缩放、旋转；
- settings 是普通应用，不能访问 `cutefish_core_v1`；
- 新增用户会话级 D-Bus 服务 `com.cutefish.Display`，由 session Shell 注册，
  内部再通过 `cutefish_core_v1` 调用 core：
  - GetOutputs / SetMode / SetScale / SetTransform；
  - 输出热插拔信号；
- settings 显示页调用该 D-Bus 服务；
- 首版只支持整数缩放；fractional-scale 不在首版范围。

---

## 7. 登录、锁屏和电源要点

### 7.0 注销清理

- core 记录每个 client 的 PID/socket/surface；
- 注销顺序：terminate → 超时 SIGKILL → 强制断开 → 清理残留 surface。

### 8.1 登录

```text
cutefish-shell --greeter
  → PAM helper
  → logind 创建会话
  → systemd --user
  → cutefish-shell --session
```

- core 全程不退出；
- 不继承 SDDM 逻辑；
- 自动登录使用独立配置，不再读 sddm.conf。

### 8.2 锁屏

- 锁定状态由 core 持有；
- Shell 只显示 LockShell；
- 锁屏期间普通应用不能接收输入；
- kill/Alt+F4/客户端置顶均不能绕过。

### 8.3 电源

- DPMS 走 core DisplayBackend；
- 挂起、关机、重启走 logind；
- 睡眠恢复后 core 重新初始化 backend 并通知 Shell。

---

## 8. 窗口特效与性能要点

- 窗口级模糊、圆角、阴影由 core 渲染管线实现；
- 必须有特效降级开关，关闭后 Dock/StatusBar 仍可正常显示；
- 4K 双屏 + 多窗口模糊必须进入性能基准；
- 桌面文件拖放必须作为 Spike 专项验证。

## 9. 截图与录屏要点

- 截图读取 core 最终合成帧；
- 录屏必须单独验证 PipeWire/DMA-BUF 路径；
- 4K 回读性能必须测试；
- portal 后端与 core 同进程或私有 IPC；
- 锁屏和权限状态必须由 core 决策。

---

## 10. 打包、安装器和依赖要点

### 10.0 安装器专用化

- 最终产品不保留通用 Calamares 框架；
- 固定安装流程：欢迎 → 磁盘 → 分区（默认自动，高级可选）→ 时区 → 用户 → 摘要 → 安装 → 完成；
- 固定包清单只安装 core/shell/应用；
- 不写入 SDDM、Plymouth 桌面阶段配置、X11 session；
- 无过渡期：直接从产品删除 Calamares/calamares-settings；
- 唯一安装器为 `cutefish-installer`；
- 首版只支持 UEFI；
- 完整 Job 清单、构建残留清理、polkit helper、live/target 包清单见
  `installer-simplification.md`。

### 10.1 cutefish-core 包必须先拆

必须从 `cutefish-core` 中移除：

```text
cupdatecursor
xembed-sni-proxy
sddm-helper
X11 session 文件
kwin-wayland 依赖
xwayland 依赖
```

否则 ISO 会通过依赖链重新拉回 KWin/XWayland。

### 10.2 SDDM 无过渡

- Debian 13 的 sddm 依赖 `xserver-xorg | xserver`、xauth、x11-common；
- 产品不保留 SDDM；
- LoginShell 是产品发布门槛，验收通过后才发布产品版本；
- 开发分支不得发布包含 SDDM 的过渡产品镜像。

### 10.3 Calamares 安装器

必须同步修改：

```text
displaymanager.conf
services-systemd.conf
plymouthcfg.conf
默认 desktop / session
```

否则安装出的系统会启用不存在的 SDDM/Plymouth/X11 session。

### 10.4 Debian Qt 包名

```text
运行时：
  qt6-wayland
  qt6-qpa-plugins
  libqt6gui6
  qml6-module-qtquick

构建：
  qt6-base-dev
  qt6-declarative-dev
  libwayland-dev
  libxkbcommon-dev
  libdrm-dev
  libgbm-dev
  libinput-dev
  libudev-dev
  libpam0g-dev
```

Debian 13 不存在：

```text
qt6-wayland-compositor
qt6-wayland-compositor-dev
qt6-base
```

### 10.5 Qt X11 库策略

- 原厂 Debian Qt 会通过 `qt6-base-dev → libgl-dev → libglx-dev → libx11-dev`
  引入 X11 客户端库；
- 产品只保证不运行 X server/XWayland/KWin；
- 如要求库级彻底删除 X11，需要定制 Qt 构建并单独立项。

---

## 11. 功能等价验收矩阵

以下功能必须与当前 X11 基线等价：

```text
桌面：壁纸、图标、右键菜单、多屏、文件拖放、视频壁纸
窗口：移动、缩放、最小化、最大化、关闭、焦点、Alt-Tab
Dock：应用图标、运行状态、固定、预览、关闭
状态栏：活动窗口、系统托盘、网络/音量/电池、通知、AppMenu
启动器：显示/隐藏、搜索、应用启动、固定
会话：登录、注销、自动登录、锁屏、用户切换
输入：键盘布局、鼠标、触控板、触摸屏、全局快捷键、fcitx5/ibus 输入法
显示：单/多屏、热插拔、旋转、缩放、DPMS
截图：全屏/区域/窗口、延时、剪贴板
录屏：PipeWire 路径、帧率、4K
剪贴板：普通复制、关闭应用保留、主选择、中键粘贴
DnD：跨应用拖放、拖放图标、取消
通知：定位、过期、点击跳转
托盘：SNI 注册、菜单、状态
全局菜单：Qt/GTK 应用 DBusMenu
安装器：纯 Wayland 环境安装出可启动系统
```

---

## 12. 研发前必须完成的 Spike

基础 7 项：

1. KMS/GBM buffer 上屏；
2. Qt Quick Shell 渲染，core 做最终合成；
3. Qt xdg-shell 客户端创建 surface 并显示，含桌面文件拖放验证；
4. libinput 输入从 core 到达 Shell 和应用；
5. 截图读取 core 最终合成帧，并验证 Lock 状态禁止导出；
6. Shell 崩溃后 core 保持显示并重启 Shell；
7. 睡眠恢复和 VT 切换，含 Plymouth → core 的 DRM master 交接。

安全、交互与范围 5 项：

8. xdg_popup/positioner：Qt Quick 菜单、ComboBox、ToolTip 的定位、嵌套、grab 和 dismiss；
9. 双 socket 隔离：普通应用 socket 不可见 `cutefish_core_v1`；锁屏隐私 fallback、缩略图限制；
10. `zwp_input_method_v2`：fcitx5/ibus 键盘 grab、候选窗、锁屏停止输入；
11. 窗口装饰：FishUI CSD 标题栏 + core move/resize 交互、边缘光标、双击最大化；
12. 效果管线：窗口级模糊/圆角/阴影，4K 双屏性能，特效降级开关。

以上 12 项通过前，不进入正式开发排期。KMS 相关项必须在 AMD 和 Intel 真机各至少一块复验。

---

## 13. 每个阶段的不可遗忘检查点

### 阶段 0

- [ ] 不发布产品镜像；
- [ ] 拆 cutefish-core 包；
- [ ] 修改 calamares-settings；
- [ ] 确认产品构建不包含 SDDM；LoginShell 为发布门槛；
- [ ] 建立 core/shell 工程骨架；
- [ ] CI 阻断 X11/KWin 新增符号。

### 阶段 1

- [ ] core 最小协议闭环；
- [ ] KmsBackend 和 VirtualBackend；
- [ ] Shell 全部核心 UI；
- [ ] 功能等价矩阵 P0 通过；
- [ ] 对应 X11 代码删除。

### 阶段 2

- [ ] LoginShell 可用；
- [ ] LockShell 不可绕过；
- [ ] 登录/注销不重启 core；
- [ ] 删除 SDDM。

### 阶段 3

- [ ] BootShell 只覆盖 rootfs 之后；
- [ ] initramfs splash 保留；
- [ ] ShutdownShell 稳定；
- [ ] 确认 Plymouth 仅运行于 initramfs；清理 SDDM 残留。

### 阶段 4

- [ ] 源码 X11/KWin 符号为 0；
- [ ] 无 Xorg/XWayland/KWin 进程；
- [ ] 依赖链策略明确；
- [ ] 全量回归通过。

---

## 14. 最容易遗忘的陷阱

1. 以为改 ISO 脚本就能去掉 X11，实际必须从 `cutefish-core` 包开始拆。
2. 以为删除 X11 后 SDDM 还能正常安装，实际 Debian sddm 依赖 xserver。
3. 以为阶段 3 可以覆盖 initramfs，实际 Qt compositor 无法在 root 挂载前运行。
4. 以为 Shell 放进核心更简单，实际 Shell 崩溃会带崩显示服务。
5. 以为用 QtWaylandCompositor 能快速落地，实际 QWaylandOutput 与 QWindow/QPA 深度绑定。
6. 以为 eglfs_kms 可以当作桌面合成器后端，实际它是嵌入式单应用 QPA。
7. 以为“无 X11 依赖”等于原厂 Qt 无 libx11，实际 qt6-base-dev 会间接引入 libx11-dev。
8. 以为剪贴板继续用 cutefish-clipboard 客户端即可，实际 Wayland 下需要 core 管理。
9. 以为通知窗口还能自己定位，实际 Wayland toplevel 不能自定位。
10. 以为视频壁纸只是换窗口类型，实际需要重写为 Shell/core 原生视频层。
11. 以为 AppMenu 删掉 xcb 就行，实际还需重做 qt-plugins 注册路径。
12. 以为安装器不归显示系统管，实际 calamares 会写出旧的 SDDM/Plymouth/会话配置。

---

## 15. 文档关联

- `cutefish-dedicated-compositor-architecture.md`
- `wayland-migration-master-plan.md`
