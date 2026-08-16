# CutefishOS 专用 Wayland 合成器最终架构

> 文档状态：V2.0，待评审
> 开发意图：只为 CutefishOS 内部版本服务，不追求通用桌面能力。
> 架构结论：**一个常驻最小核心合成器 + 一个可重启 Qt Quick Shell。**
> 设计依据：已合并 macOS / Windows 桌面架构评审结论，不再单独维护评审文档。

---

## 0. 未来修改意图：四原则与迁移决策

### 0.0 最高准则：专用、简洁、稳定、可替换

四原则是整个 CutefishOS 桌面开发的最高准则和长期指导方针，适用于桌面、Shell、
合成器、安装器、系统服务及所有后续模块，不只适用于本次迁移。

### 0.1 专用

- 只为 CutefishOS 内部版本服务；
- 只为内部 Qt/QML 应用服务；
- 不为第三方桌面、插件、脚本、主题、XWayland 服务。

### 0.2 简洁

- 进程数、模块数、协议数只减不增；
- 不做可插拔合成器框架；
- 不做通用协议矩阵；
- 新增任何能力必须先证明是当前产品必需。

### 0.3 稳定

- 核心显示服务必须跨登录、注销、Shell 崩溃、应用崩溃持续运行；
- Shell 不持有 DRM/input 权限；
- 高风险 UI 必须可重启，且重启不影响显示输出；
- 状态机变更必须可审计、可恢复。

### 0.4 可替换

- 可替换的是 backend、渲染器、协议实现，不是整体架构；
- 替换点必须接口化；
- `cutefish_core_v1` 协议必须版本化；
- 替换任一实现不得要求重写 Shell 或应用。

### 0.5 迁移衍生决策：无过渡期

“无过渡期”不是最高准则，而是四原则在本次 X11/KWin → Wayland/Qt 迁移中的
一个衍生决策，适用边界只限于本次迁移：

- 产品不发布任何“新旧并存”的迁移过渡版本；
- X11/KWin/SDDM/Calamares/Plymouth 桌面运行链不在迁移中过渡保留；
- 旧实现只存在于 git 历史，不作为产品构建目标或运行时回退；
- 每个模块直接切换到其 Wayland/Qt 最终实现，验收通过后才进入产品版本；
- 未通过验收前不发布产品版本，而不是发布一个混合版本。

---

## 1. 总体架构

```text
开机到关机：
  cutefish-compositor-core（常驻，特权）

登录前：
  cutefish-shell --greeter

登录后：
  cutefish-shell --session

应用：
  filemanager / settings / terminal / ...
```

```text
┌────────────────────────────────────────────────────────────┐
│ cutefish-compositor-core                                   │
│                                                            │
│  backend/    KmsBackend / VirtualBackend                   │
│  input/      LibinputBackend                               │
│  session/    LogindSession                                 │
│  wayland/    libwayland-server + 最小协议 + cutefish_core_v1│
│  render/     Qt Quick 场景由 Shell 渲染，核心做最终合成     │
│  wm/         窗口模型、焦点、z-order、窗口管理策略           │
│  portal/     截图、录屏授权                                 │
│  auth/       PAM helper                                    │
│  lifecycle/  Boot → Login → Session → Lock → Shutdown      │
└────────────────────────────────────────────────────────────┘
                          │ cutefish_core_v1
                          ▼
┌────────────────────────────────────────────────────────────┐
│ cutefish-shell（可信 Qt Quick 客户端，可重启）               │
│                                                            │
│  LoginShell / LockShell / SessionShell                     │
│  Desktop / Dock / StatusBar / Launcher                     │
│  BootShell / ShutdownShell                                 │
│  特效、动画、通知 UI                                        │
└────────────────────────────────────────────────────────────┘
                          │ xdg-shell / 标准 Wayland
                          ▼
                   Cutefish 内部 Qt 应用
```

---

## 2. macOS / Windows 评审结论

macOS 的 WindowServer、Windows 的 DWM 之所以稳定，核心原因相同：

- 一个系统所有的常驻合成器，不被应用替代；
- 登录 UI、桌面 Shell 是可信客户端，不是合成器插件；
- Shell 崩溃不会带崩显示服务；
- 普通应用没有窗口管理、截图、全局输入权限；
- 驱动通过稳定接口隔离，第三方插件生态不进入合成器。

Cutefish 采用同一模型：常驻核心 + 可重启 Qt Quick Shell。

## 3. 与 macOS / Windows 的对应关系

| macOS / Windows | Cutefish |
|---|---|
| WindowServer / DWM | `cutefish-compositor-core` |
| loginwindow / LogonUI | `cutefish-shell --greeter` |
| Dock / 控制中心 / Explorer | `cutefish-shell --session` |
| AppKit / Win32 应用 | Cutefish Qt Wayland 应用 |
| 系统 UI 私有通道 | `cutefish_core_v1` |

核心从开机到关机常驻；登录、注销只切换 Shell 进程，不切换显示服务。

---

## 4. 为什么核心与 Shell 分离

1. Shell QML 崩溃不能带崩显示服务；
2. 登录/注销不需要重新初始化 DRM；
3. 窗口管理策略、锁屏安全、截图授权必须由核心所有；
4. Shell 可以随时替换版本而不影响 compositor core；
5. 更接近 macOS/Windows 的稳定模型。

代价：

- 从 1 个进程变为 2 个长期进程；
- 需要维护 `cutefish_core_v1` 私有协议。

这是为了稳定原则接受的最小复杂度。

---

## 5. 技术底座决策

### 5.1 不用 QtWaylandCompositor

理由：

- `QWaylandOutput` 与 `QWindow`/Qt QPA 深度绑定，难以替换为自研 KMS 后端；
- QtWaylandCompositor 缺少生产合成器需要的 KMS/logind 集成、窗口级特效和协议控制；
- 其输出模型适合嵌套合成器或参考实现，不适合作为本产品的系统显示核心；
- KWin 也未使用 QtWaylandCompositor，而是自建 Wayland 协议和后端。

### 5.2 不用 eglfs_kms / wlroots

- eglfs_kms 是嵌入式单应用 QPA，不提供桌面合成器后端能力；
- wlroots 可作后备，但会引入新的 C 合成器框架，与“专用、简洁、可控”目标冲突。

### 5.3 不实现 wlr-layer-shell 的取舍

- 专用系统不接受第三方面板/桌面小工具生态；
- Shell surface 层级由 `bind_shell_surface` 和 `cutefish_core_v1` 私有协议承担；
- 这是与“专用”原则一致的产品取舍，已在架构中固化。

### 5.4 GTK 应用边界

- 功能矩阵中 GTK 应用仅限明确白名单；
- 首版接受 GTK 原生 CSD，不承诺与 Cutefish 视觉完全一致；
- 窗口装饰和交互由 F1 策略覆盖。

## 6. 核心模块设计

```text
cutefish-compositor-core/
├── main.cpp
├── backend/
│   ├── display_backend.h
│   ├── kms_backend.h/cpp
│   └── virtual_backend.h/cpp
├── input/
│   ├── input_backend.h
│   └── libinput_backend.h/cpp
├── session/
│   ├── session_backend.h
│   └── logind_session.h/cpp
├── wayland/
│   ├── server.h/cpp
│   ├── protocol/*.xml
│   ├── xdg_shell.cpp
│   ├── seat.cpp
│   ├── data_device.cpp
│   ├── dmabuf.cpp
│   └── cutefish_core_v1.cpp
├── render/
│   ├── compositor.h/cpp
│   ├── buffer.h/cpp
│   └── final_presenter.h/cpp
├── wm/
│   ├── window.h/cpp
│   ├── workspace.h/cpp
│   └── policy.h/cpp
├── lifecycle/
│   ├── state_machine.h/cpp
│   └── shell_manager.h/cpp
├── portal/
│   ├── screenshot.cpp
│   └── screencast.cpp
└── auth/
    ├── pam_helper.cpp
    └── session_launcher.cpp
```

### 4.1 核心职责

- DRM/KMS、GBM buffer、page flip；
- libinput 输入和 seat；
- logind 会话、VT、睡眠恢复；
- 最终合成和应用窗口合成；
- 焦点、z-order、窗口状态策略；
- 锁屏状态机；
- 截图/录屏授权；
- Shell 生命周期和崩溃恢复。

### 4.2 核心不做什么

- 不加载业务 QML；
- 不实现 Dock/启动器动画；
- 不处理通知内容；
- 不做通用插件/脚本。

---

## 7. Shell 模块设计

```text
cutefish-shell/
├── main.cpp
├── shell_client.h/cpp
└── qml/
    ├── BootShell.qml
    ├── LoginShell.qml
    ├── LockShell.qml
    ├── SessionShell.qml
    ├── ShutdownShell.qml
    ├── Desktop.qml
    ├── Dock.qml
    ├── StatusBar.qml
    └── Launcher.qml
```

### 5.1 Shell 职责

- 所有可见品牌 UI；
- 桌面、Dock、状态栏、启动器；
- 登录、锁屏、关机界面；
- Shell 自身 UI 动画；
- 通知 UI；
- 显示 core 导出的窗口元数据和缩略图。

### 5.2 Shell 不做什么

- 不打开 DRM；
- 不处理 libinput 设备；
- 不执行 PAM；
- 不能截图其他应用；
- 不能注入全局输入；
- 不能覆盖锁屏；
- 不能直接读取普通应用窗口纹理；
- 窗口级模糊、阴影、圆角由 core 执行，Shell 只提交特效参数。

---

## 8. `cutefish_core_v1` 私有协议

核心与 Shell 之间只使用版本化私有协议：

```text
cutefish_core_v1
├── get_outputs / set_output_config
├── get_windows / window_focus_changed
├── request_activate / close / minimize / maximize
├── set_dock_geometry / set_panel_geometry
├── show_launcher / hide_launcher
├── notification_anchor
├── screenshot_request / screenshot_result
├── lock / unlock_state
├── request_window_thumbnail / thumbnail_ready
├── set_window_effect_params
│     blur_radius / corner_radius / shadow / opacity
├── bind_shell_surface
│     wl_surface, role, output, z_order
├── surface_role_changed
└── shell_ready / shell_ping / shell_restart
```

Shell surface role 是强制的：

```text
Boot / Login / Lock / SessionShell / Desktop / Dock / StatusBar / Launcher
```

core 根据 role 决定层级、输出、锁屏时保留哪些 surface；Shell 重启后旧 surface 必须清理。

规则：

- 协议 XML 带版本号；
- Shell 启动时协商版本；
- 旧协议保留一个兼容周期后删除；
- 应用永远不能访问 `cutefish_core_v1`。

访问控制实现（F3 修正，双 socket 为架构必须项）：

- core 创建两个 Wayland socket：
  - `cutefish-shell.sock`：仅 shell 可连接，注册 `cutefish_core_v1`；
  - `cutefish-apps.sock`：普通应用连接，不注册 `cutefish_core_v1`；
- 不依赖 UID/PID/exe 校验作为主要隔离手段；
- shell socket 使用受限文件权限，并由 core 校验连接方；
- 锁屏期间窗口缩略图和纹理导出接口自动禁用；
- 所有帧导出接口在 Lock 状态下一律禁用，包括 portal。

`screenshot_request/screenshot_result` 限制：

- 仅 Shell 自身 UI 和 portal 预览在非锁屏状态可用；
- 结果不得包含普通应用窗口纹理；
- 普通应用窗口截图必须走 portal 的显式授权流程；
- Lock 状态下，私有协议和 portal 的所有帧导出/截图接口一律禁用。

普通应用的输出管理通路：

- settings 是普通应用，不能 bind `cutefish_core_v1`；
- 由 session Shell 在用户会话总线上注册 `com.cutefish.Display`；
- Shell 内部再通过 `cutefish_core_v1` 调用 core；
- 接口：GetOutputs / SetMode / SetScale / SetTransform / 热插拔信号；
- 不采用 root core 直接接入用户 session bus 的方案。

### 7.1 普通用户服务控制面

root core 不接入用户 session bus。所有用户会话服务通过 Shell 或可信 helper 转发：

| 服务/应用 | 会话侧接口 | 最终执行者 |
|---|---|---|
| settings 显示页 | `com.cutefish.Display` | Shell → core |
| core/powerman | `com.cutefish.Power`（Shell 注册） | Shell → core / logind |
| core/settings-daemon 输入 | `com.cutefish.Input`（Shell 或 helper 注册） | 写配置 → core 重载 libinput |
| settings 光标主题 | `com.cutefish.Cursor`（Shell 注册） | Shell → core 重载光标 |
| portal Screenshot | session bus portal 后端（helper 注册） | helper → 受控 IPC → core |
| portal ScreenCast | session bus portal 后端 + 用户 PipeWire | helper/PipeWire → core 帧导出 |
| PipeWire | 仅存在于用户会话 | 不进入 root core 进程 |

规则：

- 用户会话 D-Bus 服务不得由 root core 注册；
- 需要访问 core 的 helper 必须使用受控 IPC；
- helper 权限只覆盖其职责，不拥有完整 `cutefish_core_v1` 权限。

---

## 9. 最小 Wayland 协议集

面向应用只实现：

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
```

首版明确：

- 整数缩放；fractional-scale 不在首版协议集；
- 输入法为 P0：必须实现 `zwp_input_method_v2`，支持 fcitx5/ibus；
- IME 候选窗定位、键盘 grab、锁屏时 IME 停止输入必须定义；
- 手写笔/手势首版裁剪，不作为功能矩阵验收项。

协议来源：

- `wayland-protocols` 提供 xdg-shell、linux-dmabuf、primary-selection、text-input、xdg-activation 等 XML；
- `zwlr_data_control_v1` 属于 wlr-protocols，Debian 13 无对应包，必须 vendored XML。

明确不实现：

```text
XWayland
wlr-layer-shell
plasma-window-management
KWayland AppMenu
```

---

## 10. 窗口装饰策略（F1 专项）

首版决策：**Cutefish Qt 应用采用统一 CSD；core 不渲染标题栏纹理。**

- FishUI 提供统一 `Cutefish.Window/TitleBar` QML 组件；
- 应用通过 xdg_toplevel `move`/`resize` 请求发起交互；
- core WM 实现交互式移动/缩放状态机、边缘命中、光标形状；
- 标题栏双击最大化、按钮行为由 FishUI 组件实现；
- GTK 应用建立白名单，首版接受其原生 CSD，不保证视觉完全一致；
- 若后续 GTK 应用成为产品刚需，再评估 core SSD fallback，不进入首版范围。

## 11. 生命周期状态机

```text
Boot
  → Login
  → Session
      ⇄ Lock
  → Shutdown
```

首版决策：**不实现并发多用户会话。** “切换用户”定义为注销当前用户并回到 LoginShell。

- 状态由核心持有；
- Shell 只请求状态，不拥有状态；
- Shell 崩溃时核心保持当前状态并重启 Shell；
- 登录/注销不重启核心；
- core 记录每个 client 的 PID/socket/surface；
- 注销顺序：terminate → 超时 SIGKILL → 强制断开 → 清理全部残留 surface；
- 进入 Lock 后：
  - core 停止合成所有普通应用 surface；
  - LockShell 尚未就绪时，core 显示自身维护的安全 fallback 帧；
  - 普通客户端不能接收输入；
  - 窗口缩略图/纹理导出接口禁用。

---

## 12. 剪贴板缓存策略（H7 修正）

- core 不只转发 selection，必须缓存当前 selection 内容；
- 缓存范围：文本、图片、文件列表 URI；
- 源客户端退出后缓存仍然有效；
- 主选择缓存独立于普通剪贴板缓存；
- 设置大小上限和清理策略，用户注销时清空；
- 不进入通用剪贴板管理器设计，只做最小可靠缓存。

---

## 13. 关键接口

### DisplayBackend

```cpp
struct Output {
    QString name;
    QList<QSize> modes;
    QSize currentMode;
    int scale = 1;
    int transform = 0;
    bool connected = false;
};

class DisplayBackend {
public:
    virtual QList<Output> outputs() = 0;
    virtual bool setMode(const QString &name, const QSize &mode) = 0;
    virtual Buffer *createBuffer(const QSize &size, BufferFormat format) = 0;
    virtual void present(Output *output, Buffer *buffer) = 0;
    virtual void setDpms(const QString &name, bool on) = 0;
    virtual void setCursor(const QImage &image, const QPoint &hotspot) = 0;
};
```

### InputBackend

```cpp
class InputBackend {
public:
    virtual void keyboardKey(uint32_t key, bool pressed) = 0;
    virtual void pointerMotion(double x, double y) = 0;
    virtual void pointerButton(uint32_t button, bool pressed) = 0;
    virtual void touchDown(int id, double x, double y) = 0;
    virtual void touchMotion(int id, double x, double y) = 0;
    virtual void touchUp(int id) = 0;
    virtual void setKeyboardLayout(const QString &layout) = 0;
};
```

### SessionBackend

```cpp
class SessionBackend {
public:
    virtual bool takeControl() = 0;
    virtual bool releaseControl() = 0;
    virtual void pause() = 0;
    virtual void resume() = 0;
    virtual void switchToVT(int vt) = 0;
    virtual void lock() = 0;
    virtual void shutdown() = 0;
};
```

### ShellManager

```cpp
class ShellManager {
public:
    virtual void startGreeterShell() = 0;
    virtual void startSessionShell(uid_t uid) = 0;
    virtual void restartShell() = 0;
    virtual void stopShell() = 0;
    virtual ShellState state() const = 0;
};
```

---

## 14. 渲染与特效数据流

```text
应用窗口：
  dmabuf / shm buffer
    → core 导入纹理
    → core 执行窗口级模糊、圆角、阴影、动画
    → core 最终合成
    → GBM buffer
    → DRM page flip

Shell 自身 UI：
  Shell 进程使用标准 Qt Wayland QPA + QWindow
    → 普通 xdg-shell surface
    → core 作为特殊客户端合成

窗口缩略图 / Dock 预览 / Alt-Tab：
  Shell 通过 cutefish_core_v1 请求窗口缩略图
    → core 从应用窗口纹理生成
    → 通过版本化接口导出给 Shell
    → 锁屏/隐私状态下禁止导出
```

边界：

- core 不加载业务 QML；
- Shell 不直接访问应用窗口纹理；
- 首版 Shell 使用标准 Qt Wayland QPA，不使用 QQuickRenderControl；
- 如未来 Shell 需要自渲染，再单独设计 buffer 提交路径；
- 特效必须有全局降级开关，关闭后 dock/statusbar 仍可正常显示；
- 4K 双屏 + 多窗口模糊必须进入性能基准。

---

## 15. 崩溃与恢复策略

| 故障 | 策略 |
|---|---|
| Shell 崩溃 | 非锁屏时保留最后一帧缓存或显示品牌背景；锁屏时显示 core 安全 fallback；随后重启 Shell |
| 应用崩溃 | 关闭应用窗口，不影响核心和 Shell |
| 核心崩溃 | systemd 重启核心，恢复 Boot/Login 状态 |
| GPU reset | 核心重建 render/backend，通知 Shell 刷新 |
| 锁屏 Shell 未就绪/崩溃 | 核心显示安全 fallback 帧并保持锁定，不合成普通应用 |
| 锁屏无响应 | 核心保持锁定，强制重启 Shell |

---

## 16. 最终依赖

保留：

```text
Qt Core / Gui / Quick
libwayland-server
libdrm
libgbm
libinput
libxkbcommon
libudev
PAM
systemd/logind
Mesa/EGL
```

禁止：

```text
KWin
KDE Plasma
KScreen
KWindowSystem
QtWaylandCompositor API 链接与调用
eglfs_kms
wlroots
XWayland
```

Debian 打包现实：

- `qt6-wayland` 客户端插件依赖 `libqt6waylandcompositor6`，因此原厂 Debian 镜像中
  可能存在 QtWaylandCompositor 库文件；
- 产品约束是：**不链接、不调用 QtWaylandCompositor API**；
- 是否通过定制 Qt 移除该库，由阶段 4 依赖策略决定。

---

## 17. 规模定位

规模初估仅作立项参考，正式排期必须等 Spike 数据。第五轮评审后按
“含 CSD 交互、input-method、效果管线、剪贴板缓存、双 socket、portal”重新估算：

```text
core：预计 4.0 万–6.0 万行 C++
shell：预计 1.0 万–2.0 万行 QML/C++
合计：约 5.0 万–8.0 万行
```

这不是通用合成器，而是 CutefishOS 的专用系统显示服务。阶段排期只作为预估，
以 Spike 实测后的里程碑估算为准。

---

## 18. 评审前必须完成的 Spike

基础 7 项：

1. KMS/GBM buffer 上屏；
2. Qt Quick Shell 渲染，核心做最终合成；
3. Qt xdg-shell 客户端创建 surface 并显示，含桌面文件拖放验证；
4. libinput 输入从核心到达 Shell 和应用；
5. 截图读取核心最终合成帧，并验证 Lock 状态禁止导出；
6. Shell 崩溃后核心保持显示并重启 Shell；
7. 睡眠恢复和 VT 切换，含 Plymouth → core 的 DRM master 交接。

安全、交互与范围 5 项：

8. xdg_popup/positioner：Qt Quick 菜单、ComboBox、ToolTip 的定位、嵌套、grab 和 dismiss；
9. 双 socket 隔离：普通应用 socket 不可见 `cutefish_core_v1`；锁屏隐私 fallback、缩略图限制；
10. `zwp_input_method_v2`：fcitx5/ibus 键盘 grab、候选窗、锁屏停止输入；
11. 窗口装饰：FishUI CSD 标题栏 + core move/resize 交互、边缘光标、双击最大化；
12. 效果管线：窗口级模糊/圆角/阴影，4K 双屏性能，特效降级开关。

以上 12 项通过后，方可进入正式开发排期。KMS 相关项必须在 AMD 和 Intel 真机各至少一块复验。
