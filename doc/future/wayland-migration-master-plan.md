# CutefishOS 专用 KMS + Qt Wayland 合成器总体方案

> 文档状态：V4.1 最终架构优化为“常驻核心合成器 + 可重启 Qt Quick Shell”，待评审
> 调研基准：`cutefish_project` 当前 main 分支及子模块 HEAD
> 测试服务器：192.168.118.132（Debian 13，当前生产会话仍为 X11，仅作为迁移前基线）
> 产品定位：内部版本，不保留 X11、不依赖 KWin/KDE 桌面栈、不安装 XWayland。
> 核心诉求：**在功能完整的前提下，用 Qt/Wayland 原生能力替换 X11 和 KWin。**
> 最高准则：**专用、简洁、稳定、可替换**，是整个 CutefishOS 桌面开发的长期指导方针。
> 迁移决策：**无过渡期**，是四原则在本次 X11/KWin → Wayland/Qt 迁移中的衍生决策，
> 产品不发布新旧并存的迁移过渡版本；旧实现只存在于 git 历史。
> 关键文档：`cutefish-dedicated-compositor-architecture.md` 为最终架构和实施依据；
> `development-key-points.md` 为研发重点要点与防遗忘清单；
> `installer-simplification.md` 为 Calamares 专用化精简的独立工作流。

---

## 1. 迁移目标与硬性约束

### 1.1 产品目标

1. 默认、唯一显示协议为 Wayland。
2. 最终系统：
   - 无 Xorg、无 X11 session、无 XWayland；
   - 无 KWin、无 KDE Plasma 桌面栈依赖；
  应用级 KF6 库（KIO/KConfig/Solid/SyntaxHighlighting/BluezQt/NetworkManagerQt 等）
  暂不承诺全部移除，单独评审；KScreen/KWindowSystem/Plasma 桌面栈必须移除。
   - 无 SDDM，登录/锁屏由 `cutefish-compositor` 完成；
   - 桌面运行时不运行 Plymouth；Plymouth 只作为 initramfs splash 存在，
  rootfs 后退出，compositor 负责后续 Boot/ShutdownShell。
3. 显示服务由一个常驻的 `cutefish-compositor-core` 承担；登录、桌面、Dock、状态栏、
   启动器、锁屏、开机/关机 UI 由可重启的 `cutefish-shell` 承担。
4. 所有内部应用使用 Qt Wayland 原生运行；不兼容应用重写或移出产品。
5. 不通过保留 X11 或 KWin 来保证稳定性；稳定性来自内部灰度、自动测试和镜像级回滚。
6. 不发布过渡版本：X11/KWin/SDDM/Calamares 不做产品运行时过渡；每个模块直接切换到
   最终实现，验收通过后才进入产品版本。

### 1.2 硬性约束

| 约束 | 要求 |
|---|---|
| 显示服务器 | 常驻 `cutefish-compositor-core`：libwayland-server + 自研 KMS + 最终合成 |
| 应用协议 | 只实现 Cutefish 需要的 xdg-shell 最小协议集 |
| XWayland | 不安装、不启动 |
| 登录 | `cutefish-shell --greeter` + PAM helper + logind |
| 锁屏 | core 维护锁定状态，`cutefish-shell` 显示 LockShell |
| 开机/关机 | `cutefish-shell` 的 Boot/ShutdownShell；core 常驻 |
| 源码 | 最终无 X11/XCB/Xlib 功能代码 |
| 构建 | Wayland-only，无 KWin/KDE 桌面栈开发包；应用级 KF6 依赖单独列清单 |
| 阶段 0 | 工程基线，不发布产品镜像；X11/KWin/SDDM/Calamares 只存在于 git 历史，不作为产品运行时 |
| 回退 | 镜像/软件版本回退，不做 X11 或 KWin 运行时回退 |
| Qt X11 库 | 原厂 Debian Qt 仍携带 libx11/libxcb；产品只禁止运行 X server/XWayland/KWin |

---

## 2. 当前架构与 X11 基线

### 2.1 当前进程拓扑

```text
当前基线（仅用于审计）：
SDDM → Xorg → kwin_x11 → cutefish-session
  ├─ settings-daemon / powerman / clipboard / notificationd / polkit
  ├─ chotkeys（XGrabKey）
  ├─ gmenuproxy（xcb 全局菜单）
  ├─ xembed-sni-proxy（XEmbed 托盘）
  ├─ desktop / dock / statusbar / launcher
  └─ Qt/QML 应用

目标：
cutefish-compositor-core（常驻特权核心）
  ├─ 自研 KMS/GBM、libinput、logind
  ├─ 最小 Wayland 协议 + cutefish_core_v1
  ├─ 窗口管理、最终合成、锁屏状态、截图授权
  └─ 原生 Wayland 应用（最小 xdg-shell）

cutefish-shell（可信 Qt Quick 客户端，可重启）
  ├─ BootShell / LoginShell / SessionShell / LockShell / ShutdownShell
  ├─ Desktop / Dock / StatusBar / Launcher
  └─ 特效、动画、通知 UI
```

### 2.2 已有 Wayland 雏形

当前 `core/session` 已实现 `kwin_wayland` 启动和 Wayland session 文件，但该雏形依赖
KWin，且存在以下问题：

- 初始化无条件调用 `xrdb`、`cupdatecursor`；
- `cutefish-gmenuproxy` 强制 `QT_QPA_PLATFORM=xcb`；
- dock/statusbar 大量调用 `KX11Extras`；
- 截图使用 `QScreen::grabWindow(0)`，Wayland 下黑图；
- KWin 启动等待用 900ms sleep；
- KWin 裸会话缺 Plasma QML 模块，effect 脚本报错。

在 V4.1 架构下，这些 X11 路径和 KWin 路径都不保留，全部替换或删除。

---

## 3. X11/KWin 依赖审计与删除计划

### 3.1 审计结论

X11 依赖最深的是 session、Shell、输入设备、托盘、快捷键、截图和视频壁纸。
KWin 依赖集中在会话启动、窗口管理、特效、锁屏和 KWin 插件。
纯 Qt 路线下，两者都删除：

| 模块 | X11/KWin 依赖 | 纯 Qt 替代 |
|---|---|---|
| `core/session` | xcb/EWMH、KWin 启动 | compositor 生命周期；session 只负责应用和环境 |
| `desktop` / `dock` / `statusbar` / `launcher` | KX11Extras、KWindowSystem | QML 迁入 `cutefish-shell`，窗口模型/输出/特效参数经 `cutefish_core_v1` |
| `core/chotkeys` | XGrabKey | compositor 输入分发前处理全局快捷键 |
| `core/gmenuproxy` | xcb appmenu 属性 | 删除或重写为普通 D-Bus 服务；AppMenu 数据走用户会话 Registrar，DBusMenu 数据面保留 |
| `core/settings-daemon` | XInput2/Synaptics | libinput 配置 + compositor 配置接口 |
| `core/powerman` | xcb/dpms | compositor DRM/DPMS + logind |
| `core/cupdatecursor` | Xcursor | compositor 光标主题管理，组件删除 |
| `core/xembed-sni-proxy` | XEmbed | SNI only，组件删除 |
| `screenshot` | X11 grabWindow | compositor 内抓帧 + portal |
| `screenlocker` | X11 宏、旧客户端模型 | compositor LockShell |
| `fishui` | X11 窗口移动/模糊 | compositor 内特效；客户端只用 QWindow API |
| `qt-plugins` | X11 integration | 删除 X11 integration |
| `fantascene-dynamic-wallpaper` | xcb-ewmh | compositor/desktop 原生视频壁纸 |
| `kwin-plugins` | kwin-x11、KWin effect | 整个模块不再使用 |
| `sddm-theme` / `plymouth-theme` | SDDM/Plymouth | compositor Login/Boot/ShutdownShell |

### 3.2 必须删除的第三方桌面依赖

```text
kwin-x11
kwin-wayland
kwin-common
libkdecorations3-6
libkf6windowsystem6（Cutefish 直接使用部分）
KScreenLocker（仅锁屏相关）
PlasmaWindowManagement / KWayland AppMenu（按需可保留协议 XML，但不依赖 KDE 运行库）
```

---

## 4. 目标架构：常驻核心合成器 + 可重启 Qt Quick Shell

### 4.1 总体架构

```text
cutefish-compositor-core（常驻，特权）
  ├── KmsBackend / VirtualBackend
  ├── LibinputBackend
  ├── LogindSession
  ├── libwayland-server + 最小协议 + cutefish_core_v1
  ├── 最终合成与上屏
  ├── 窗口管理、焦点、z-order
  ├── 锁屏状态机
  ├── 截图/录屏授权
  └── Shell 生命周期管理

cutefish-shell（可信 Qt Quick 客户端，可重启）
  ├── BootShell / LoginShell / SessionShell / LockShell / ShutdownShell
  ├── Desktop / Dock / StatusBar / Launcher
  └── 特效、动画、通知 UI

Cutefish 应用（普通 xdg-shell 客户端）
```

核心从开机到关机常驻；登录、注销只切换 Shell 进程，不重启显示服务。

### 4.2 与 macOS / Windows 的对应

| macOS / Windows | Cutefish |
|---|---|
| WindowServer / DWM | `cutefish-compositor-core` |
| loginwindow / LogonUI | `cutefish-shell --greeter` |
| Dock / 控制中心 / Explorer | `cutefish-shell --session` |
| AppKit / Win32 应用 | Cutefish Qt Wayland 应用 |

### 4.3 技术底座决策

- 不用 QtWaylandCompositor；
- 不用 eglfs_kms；
- 不用 wlroots；
- 协议层用 libwayland-server，只实现 Cutefish 需要的协议；
- Qt 只负责 Shell QML、控件、D-Bus 和应用框架；
- DRM、libinput、logind、最终合成全部在核心自研。

### 4.4 生命周期

```text
Boot → Login → Session ⇄ Lock → Shutdown
```

- 状态由核心持有；
- Shell 只请求状态，不拥有状态；
- Shell 崩溃由核心重启，不重启显示服务；
- 登录/注销不切换 DRM。

### 4.5 最小 Wayland 协议集

```text
wl_compositor / wl_subcompositor / wl_shm / wl_seat / wl_output
zxdg_output_manager_v1
wl_data_device_manager
wp_viewporter
xdg_wm_base / xdg_surface / xdg_toplevel / xdg_popup
wp_primary_selection_v1
zwlr_data_control_v1
zwp_linux_dmabuf_v1
zwp_text_input_manager_v3
zwp_input_method_v2
xdg_activation_v1
cutefish_core_v1       # 仅 Shell 可信客户端可用
```

不实现：XWayland、wlr-layer-shell、PlasmaWindowManagement、KWayland AppMenu。

### 4.6 可替换接口

```text
DisplayBackend     # KMS 后端
InputBackend       # libinput
SessionBackend     # logind
ShellManager       # Shell 启动/重启/切换
Renderer           # 核心最终合成
```

### 4.7 最终依赖边界

保留：Qt Core/Gui/Quick、libwayland-server、libdrm、libgbm、libinput、
libxkbcommon、libudev、PAM、systemd/logind、Mesa/EGL。

禁止：KWin、KDE Plasma、KScreen、KWindowSystem、QtWaylandCompositor、
eglfs_kms、wlroots、XWayland。

详细模块目录、接口、私有协议和开发意图见
`cutefish-dedicated-compositor-architecture.md`；
macOS/Windows 评审结论已合并进 `cutefish-dedicated-compositor-architecture.md`。

---

## 4.8 独立工作流：安装器专用化精简

Calamares 是通用安装框架，不符合“专用、简洁、稳定、可替换”原则。本迁移中将其
作为独立工作流处理，与显示系统迁移并行：

```text
Calamares + calamares-settings
  → 直接从产品删除，不保留过渡版本
  → 唯一安装器：cutefish-installer（Qt Quick + FishUI + polkit helper）
```

固定安装流程：

```text
品牌欢迎页 → 磁盘选择 → 分区方式（默认自动，高级分区可选）
→ 时区选择 → 用户创建 → 摘要确认 → 执行安装 → 完成重启
```

固定包清单只安装：

```text
cutefish-compositor-core
cutefish-shell
cutefish-core（Wayland-only 拆包后的用户会话核心服务）
qt6-wayland
Cutefish 应用
```

不再写入 SDDM、Plymouth 桌面阶段配置、X11 session。

首版安装器只支持 UEFI；完整 Job 清单、polkit helper、live/target 包清单见
`installer-simplification.md`。

---

## 5. 实施阶段

每个阶段有独立目录和详细文档：

| 阶段 | 目录 | 目标 |
|---|---|---|
| 阶段 0 | `stage-0-wayland-only-baseline/` | 包依赖解耦，建立专用 compositor 工程与虚拟后端 |
| 阶段 1 | `stage-1-qt-compositor-shell/` | 自研 KMS + 协议 + 渲染 + Shell/WM 完整闭环 |
| 阶段 2 | `stage-2-login-lock/` | LoginShell/LockShell，去掉 SDDM |
| 阶段 3 | `stage-3-boot-shutdown/` | Boot/ShutdownShell；Plymouth 缩减为 initramfs splash |
| 阶段 4 | `stage-4-x11-purge/` | X11 和 KWin 残留彻底清除 |

### 5.1 阶段 0：工程基线与包依赖解耦（1–2 周，不发布产品镜像）

- 本阶段是工程基线，不是可发布系统；不发布任何混合版本；
- 旧 X11/KWin/SDDM/Calamares 实现只保留在 git 历史，不作为产品构建目标；
- 直接修改 `cutefish-core` 包：删除 `cupdatecursor`、`xembed-sni-proxy`、
  `sddm-helper`、X11 session，移除 `kwin-wayland`/`xwayland` 依赖；
- 直接删除 `sddm_autologin.conf` 和 calamares-settings 产品配置；
- 产品登录只允许 `cutefish-shell --greeter` + PAM/logind，不保留 SDDM 路径；
- 建立 `cutefish-compositor-core` 和 `cutefish-shell` 工程：
  `VirtualBackend` 先跑协议与 QML 状态机，真机走 `KmsBackend`；
- 建立 X11/KWin 符号 CI 扫描；
- 完成评审前 12 项 Spike：基础 7 项 + xdg_popup/positioner + 双 socket/锁屏隐私 + input-method + 窗口装饰 + 效果管线；
- 生成功能等价矩阵和删除清单。

### 5.2 阶段 1：核心合成器与 Shell 完整闭环（排期待 Spike 后确定）

目标：`cutefish-compositor-core` 完成显示、输入、协议、窗口管理和最终合成；
`cutefish-shell` 作为可信 Qt Quick Wayland 客户端承载全部用户 UI。

关键工作：

1. `cutefish-compositor-core`（依赖 QtCore/QtGui/DBus，不依赖 Qt Quick）：
   - `libwayland-server` 协议层：最小 xdg-shell、seat、data-device、dmabuf、
     `cutefish_core_v1`；
   - `KmsBackend`：DRM/KMS + GBM + atomic modeset + page flip；
   - 最终合成、窗口模型、焦点、置顶、最大化/最小化/关闭；
   - 窗口级模糊、阴影、圆角、动画；
   - ShellManager 和锁屏安全 fallback。
2. `cutefish-shell`：
   - 标准 Qt Wayland QPA 客户端；
   - Boot/Login/Session/Lock/Shutdown QML；
   - Desktop/Dock/StatusBar/Launcher QML；
   - 复用 FishUI 控件和主题；
   - Shell 崩溃后由 core 重启。
3. 核心服务：
   - 全局快捷键在 compositor 输入链中实现；
   - AppMenu 数据走用户会话 D-Bus Registrar；Shell 经 `cutefish_core_v1` 获得 focused app_id 后查询 D-Bus；gmenuproxy 删除或重写为普通 D-Bus 服务。
   - 截图/录屏由 compositor 抓帧 + portal 后端；
   - 光标主题、DPMS、输入配置走 compositor。
4. X11 代码删除：
   - 每个功能验收后删除对应 X11/KWin 实现；
   - 删除 `cupdatecursor`、`xembed-sni-proxy`；
   - 删除 KX11Extras、xcb、Xlib 调用。

### 5.3 阶段 2：Login/LockShell 与去 SDDM（4–6 周）

- 先完成 4.6 节进程模型设计评审，再开始编码；
- LoginShell：用户列表、密码、会话选择、自动登录；
- PAM 认证通过独立 helper，logind 会话注册；
- 删除 SDDM 和 sddm-theme；
- 挂起、关机、重启走 logind。

### 5.4 阶段 3：Boot/ShutdownShell 与 initramfs splash 边界（4–6 周）

- compositor BootShell 只承诺 **rootfs 挂载、udev/DRM 就绪之后**；
- initramfs、fsck、root 挂载前的空窗由极简 initramfs splash/Plymouth 覆盖；
- 不承诺“GRUB→BootShell 全程无空窗”；验收改为“initramfs splash → BootShell 无闪屏”；
- ShutdownShell 必须在 systemd 停止 Qt 依赖服务前进入，并保持到关机最后阶段；
- 达到边界一致性后，Plymouth 只缩到 initramfs，不追求完全删除。

### 5.5 阶段 4：X11/KWin 彻底清除（2–3 周）

- 删除全部 X11/KWin 源码、CMake 查找、包依赖；
- 删除 X11 session、Xorg、XWayland、xterm/xinit/xauth/xrdb；
- 进程白名单禁止 Xorg/XWayland/KWin；
- 源码扫描 X11/XCB/Xlib/KWin 符号为 0。

---

## 6. 模块级迁移顺序

| 优先级 | 模块 | 原实现 | Qt-only 目标 |
|---|---|---|---|
| P0 | session | X11/KWin 启动 | compositor 生命周期管理 |
| P0 | desktop | KX11Extras Desktop | 桌面 QML 迁入 `cutefish-shell`，层级由 `cutefish_core_v1` 绑定 |
| P0 | dock | KX11Extras + strut | Dock QML 迁入 `cutefish-shell`，窗口模型来自 core |
| P0 | statusbar | KX11Extras + xcb appmenu | 顶栏 QML 迁入 `cutefish-shell`，AppMenu 走用户 D-Bus |
| P0 | launcher | X11 tooltip 假设 | Launcher QML 迁入 `cutefish-shell` |
| P0 | screenshot | grabWindow | compositor 抓帧 + portal |
| P0 | chotkeys | XGrabKey | compositor 快捷键链 |
| P0 | gmenuproxy | 强制 xcb | 删除或重写为普通 D-Bus 服务，AppMenu 走用户会话 Registrar |
| P1 | settings-daemon | XInput2/Synaptics | libinput + compositor 配置 |
| P1 | powerman | X DPMS | compositor DRM/DPMS + logind |
| P1 | screenlocker | 旧客户端 | compositor LockShell |
| P1 | fishui | X11 helper | compositor effects + Qt QWindow API |
| P1 | qt-plugins | X11 integration | 删除 X11 integration |
| P1 | cursor | Xcursor | compositor 光标管理 |
| P1 | kwin-plugins | KWin effect | 删除模块，特效进 compositor |
| P2 | cupdatecursor | X11 命令 | 删除组件 |
| P2 | xembed-sni-proxy | XEmbed | 删除组件，SNI only |
| P2 | fantascene | X11 视频壁纸 | compositor/desktop 视频壁纸 |
| P2 | SDDM/Plymouth | 独立 DM/动画 | compositor Login/Boot/ShutdownShell |
| P2 | 构建链 | Xorg/KWin 包 | Qt Wayland-only 包清单 |

---

## 7. 功能保证策略

### 7.1 功能等价矩阵

以 X11 基线功能为验收单位：

| 功能域 | 必须覆盖 |
|---|---|
| 桌面 | 壁纸、图标、多屏、右键菜单、文件拖放、视频壁纸 |
| 窗口管理 | 标题栏、移动/缩放/最小化/最大化/关闭、任务栏、焦点、分组、特效 |
| 面板 | 顶栏占位、应用菜单、系统托盘、网络/音量/电池、通知 |
| 启动器 | 显示/隐藏、搜索、启动、固定 |
| 会话 | 登录、注销、自动登录、锁屏、用户切换 |
| 输入 | 键盘、鼠标、触控板、触摸屏、快捷键、fcitx5/ibus 输入法 |
| 手写笔/手势 | 首版裁剪，后续单独立项 |
| 显示 | 单/多屏、热插拔、旋转、缩放 |
| 截图录屏 | 全屏/区域/窗口、延时、剪贴板 |
| 电源 | 息屏、待机、恢复、合盖、关机/重启 |
| 系统托盘 | SNI 注册、菜单、状态 |
| 全局菜单 | 内部 Qt/GTK 应用 DBusMenu |
| 剪贴板/主选择 | 关闭应用后保留剪贴板、中键粘贴 |
| DnD | 跨应用拖放、拖放图标、drop/cancel 状态 |
| 通知 | 通知弹窗定位、过期、点击跳转 |
| 输出管理 | 多屏、分辨率、缩放、旋转（替代 KScreen） |
| 任务切换 | Alt-Tab、窗口分组、任务栏同步 |
| 安装器 | Calamares 在纯 Wayland 环境安装出可启动系统 |
| 应用 | 文件管理器、设置、终端、计算器、编辑器、播放器 |

### 7.2 稳定性策略

- 不做 X11/KWin 运行时回退；
- 故障恢复：systemd 重启 compositor、会话重建、镜像回滚；
- 质量门禁：Wayland-only 构建、compositor 冒烟、X11/KWin 符号扫描、真机长稳。

---

## 8. 测试方案

- 开发/CI 使用 `VirtualBackend` 验证协议、QML 状态机和窗口管理；
- 真机使用 `KmsBackend` 验证 DRM 输出、libinput 输入和睡眠恢复；
- 覆盖冷启动、登录、锁屏、截图、多屏、睡眠恢复、GPU reset；
- 进程白名单：不允许 Xorg、XWayland、KWin。

---

## 9. 风险与应对

| 风险 | 应对 |
|---|---|
| 自研 KMS 后端复杂度 | 后端接口化；先以 VirtualBackend 验证上层，真机逐硬件验证 |
| Qt Wayland Compositor 不再使用 | 协议层用 libwayland-server 自研，只实现最小协议集 |
| Shell 集成重构大 | 分步复用 QML，先桌面再 Dock/顶栏/启动器 |
| 登录/锁屏安全 | compositor 状态机 + PAM/logind + 安全测试 |
| 截图/录屏 | compositor 抓帧，portal 与 compositor 同进程或私有 IPC |
| 无 XWayland 导致旧应用缺失 | 应用全部原生 Wayland；缺失应用重写或移出 |
| 上游 Qt 包仍含 X11 可选代码 | 产品不运行 X11；阶段 4 用定制构建关闭 |
| 项目工作量超出预期 | 以 MVP 为第一里程碑，按功能矩阵滚动交付 |
| 渲染器/后端替换成本 | 所有替换点收敛到 DisplayBackend/InputBackend/SessionBackend/Renderer |
| 剪贴板/DnD/通知等协议缺失 | 剪贴板管理进 compositor；DnD/通知定位纳入 P0 矩阵 |
| 安装器安装出不可启动系统 | 阶段 0 同步改 calamares-settings 和 core 包 |
| 原厂 Qt 依赖 X11 客户端库 | 接受“有 libx11/libxcb，但不运行 X server”，或另行定制 Qt |

---

## 10. 决策点

1. 批准最终架构：libwayland-server + 自研 KMS/GBM + Qt Quick 专用合成器。
2. 批准自研 KmsBackend，是否接受当前内部硬件范围限制？
3. 批准核心/Shell 分离：Shell 是可重启 Qt Quick 客户端，不集成进 core。
4. 批准 portal 后端由用户会话 helper 注册，经受控 IPC 转发到 core。
5. 无法原生 Wayland 的内部应用清单与处置方案。
6. 是否保留历史 X11/KWin 代码在单独 git 分支。
7. 批准阶段 0 为“工程基线，不发布产品镜像”。
8. 批准先执行 12 项 Spike，再根据结果确定正式排期；KMS 项需 AMD/Intel 真机复验。
9. 明确 KF6 边界：桌面栈必删，应用级 KF6 是否保留及保留清单。
10. 原厂 Qt X11 客户端库策略：接受库存在但禁止运行，还是自建 Qt。
11. 窗口装饰：批准首版 FishUI CSD + core move/resize；GTK 应用白名单。
12. 输入法：批准 `zwp_input_method_v2` 进入 P0 最小协议集和首版矩阵。
13. 私有协议隔离：批准双 socket 为架构必须项。
14. KMS Spike 硬件：批准 AMD/Intel 真机各至少一块复验。
15. 剪贴板持久化：批准 core 缓存文本/图片/文件 URI 及大小上限策略。
16. 安装器：批准无过渡期，直接切换到 `cutefish-installer`。
17. 批准“无过渡期”为本次迁移的衍生决策；四原则仍为整个桌面开发的最高准则。

---

## 附录 A：X11 依赖文件级清单

迁移完成后本清单应为空。

```text
core/session/CMakeLists.txt                  xcb, xcb-ewmh
core/session/application.cpp                 xrdb/cupdatecursor/X11 splash
core/session/processmanager.cpp              xcb-ewmh WM 检测/KWin 启动
core/chotkeys/hotkeys.cpp                    XGrabKey
core/powerman/dimdisplayaction.cpp           xcb/dpms
core/cupdatecursor/*                         整个组件
core/gmenuproxy/*                            xcb appmenu/强制 xcb
core/xembed-sni-proxy/*                      整个组件
core/settings-daemon/mouse/x11*              XInput2
core/settings-daemon/touchpad/x11/*          Synaptics/XRecord
desktop/desktopview.cpp                      KX11Extras
desktop-background/*                         KX11Extras
filemanager/desktop/desktopview.cpp          KX11Extras
dock/src/xwindowinterface.cpp                KX11Extras
statusbar/src/*                              KX11Extras/xcb appmenu
fishui/src/platforms/linux/*                 XOpenDisplay/xcb
qt-plugins/platformtheme/x11integration.*    xcb/NETWinInfo
screenshot/src/screenshotview.cpp            grabWindow/X11Bypass
settings/src/cursor/cursortheme.cpp          Xcursor/Xft
fantascene-dynamic-wallpaper/*               xcb-ewmh
kwin-plugins/*                               KWin effect/x11shadow
build_iso/*                                   xorg/kwin/xwayland 包与配置
```

---

## 附录 B：测试服务器实测记录

### B.1 当前基线

- 生产会话：SDDM → Xorg → `kwin_x11` → `cutefish-session`；
- 已安装 `kwin-wayland`、`qt6-wayland`、`xwayland`、`weston`；
- 该 X11/KWin 环境仅作为迁移前基线，最终全部移除。

### B.2 虚拟 Wayland 整会话测试

使用 `kwin_wayland --virtual` 对当前雏形做了隔离测试，主要结论：

- 桌面、Dock、状态栏等进程可以拉起；
- `cupdatecursor` 退出 code 2；
- `cutefish-gmenuproxy` 强制 xcb 崩溃；
- dock/statusbar 大量 `KX11Extras` 告警；
- KWin effect 脚本报错，缺 Plasma QML 模块。

这些结论进一步支持：不再修补 KWin 裸会话，直接迁移到 Qt 自研 compositor。

### B.3 截图专项

`QScreen::grabWindow(0)` 在 Wayland 下得到空/黑图，必须由 compositor 抓帧替代。

---

## 文档索引

- `cutefish-dedicated-compositor-architecture.md`：最终专用合成器架构、开发意图和 macOS/Windows 评审结论。
- `development-key-points.md`：研发重点要点清单，防止遗漏关键约束、改造点和验收项。
- `installer-simplification.md`：安装器专用化精简方案（Calamares → cutefish-installer）。
- `stage-0-wayland-only-baseline/README.md`
- `stage-1-qt-compositor-shell/README.md`
- `stage-2-login-lock/README.md`
- `stage-3-boot-shutdown/README.md`
- `stage-4-x11-purge/README.md`
