# 阶段 0：工程基线与包依赖解耦

> 所属主计划：`../wayland-migration-master-plan.md`
> 评审状态：按内部评审修订
> 定位：**本阶段是工程基线，不发布产品镜像。**
> 最高准则：专用、简洁、稳定、可替换。
> 迁移衍生决策：无过渡期。X11/KWin/SDDM/Calamares 只存在于 git 历史，不作为产品运行时。

## 1. 需要修改的项目

| 项目/路径 | 修改内容 |
|---|---|
| `cutefish/code/core` | 先拆包/停用：`cupdatecursor`、`xembed-sni-proxy`、`sddm-helper`、X11 session；移除 `kwin-wayland`/`xwayland` 依赖 |
| `cutefish/code/core/debian/control`、`CMakeLists.txt` | 重写依赖和子目录构建；Wayland-only 包不再产生上述 X11 组件 |
| `build_iso/package/cutefish/cutefish-core_*.deb` | 重新构建并验证依赖中无 `kwin-wayland`/`xwayland` |
| `build_iso/config/sddm_autologin.conf` | 删除；产品登录只走 `cutefish-shell --greeter` |
| `cutefish/code/calamares-settings` | 直接从产品删除；安装器唯一实现为 `cutefish-installer` |
| `script/create_iso/build_live_filesystem.sh` | 删除 Xorg/xterm/xinit；按阶段 0 是否保留 SDDM 决定 xserver 依赖 |
| `script/create_iso/build_install_filesystem.sh` | 同上 |
| `build_iso/script/sddm-switch-display-server.sh` | 删除 X11/Weston 切换逻辑 |
| 新增 `cutefish/code/cutefish-compositor-core` | 常驻核心合成器工程骨架、CI、systemd 草案 |
| 新增 `cutefish/code/cutefish-shell` | Qt Quick Shell 工程骨架，先跑 Boot/Login 最小界面 |
| CI/脚本 | X11/KWin 符号扫描、Qt compositor 协议冒烟 |

## 2. 依赖

### 2.1 Debian 13 正确 Qt 包名

```text
产品运行时：
  qt6-wayland                    # 内部应用使用
  qt6-qpa-plugins
  libqt6gui6
  qml6-module-qtquick
  libwayland-server0（compositor 链接）
  libdrm / libgbm / libinput 运行库

compositor 构建：
  qt6-base-dev
  qt6-declarative-dev
  libwayland-dev
  libxkbcommon-dev
  libdrm-dev
  libgbm-dev
  libinput-dev
  libudev-dev
  libpam0g-dev
  libwayland-cursor-dev
  wayland-protocols
  vendored wlr-protocols（zwlr_data_control_v1 XML）
```

说明：

- Debian 13 没有 `wlr-protocols` 包，`zwlr_data_control_v1` XML 必须 vendored；
- `qt6-wayland` 客户端插件会依赖 `libqt6waylandcompositor6`，该库可存在于镜像，
  但 Cutefish 不链接、不调用 QtWaylandCompositor API。

Debian 13 不存在 `qt6-wayland-compositor`、`qt6-wayland-compositor-dev`、`qt6-base`；最终架构也不再使用 QtWaylandCompositor。

### 2.2 SDDM 依赖事实

```text
sddm 依赖：x11-common、xauth、xserver-xorg | xserver
sddm 还链接 libxcb1 / libxcb-xkb1
SDDM Wayland greeter 默认 CompositorCommand=weston --shell=kiosk
```

因此阶段 0 必须做明确选择：

- 方案 1：暂时保留 SDDM 及其必要依赖，不宣称“ISO 无任何 X11 包”；
- 方案 2：提前去 SDDM，但阶段 2 完成前系统无登录界面，不可作为产品；
- 方案 3：重打包/定制 SDDM，固定 compositor 并剥离 xserver 依赖。

当前建议：开发分支采用方案 1，产品切换必须等阶段 2 LoginShell 验收通过。

## 3. 需要解决的问题

1. `cutefish-core` 包是依赖链阻塞点，不是只改 ISO 脚本能解决。
2. SDDM 与“删除 Xorg/xserver”在 Debian 13 包依赖上冲突。
3. `sddm_autologin.conf` 仍指向已删除的 xsession。
4. Calamares 安装器仍写 sddm.service、Plymouth 和 xsession 默认项。
5. 多个模块 CMake 强制查找 X11/XCB。
6. Qt compositor CI 无 GPU 验证方案。
7. 原厂 Debian Qt 仍携带 X11 客户端库，阶段 0 不承诺库级清除。

## 4. 功能变化

- 不发布可用的产品镜像；
- 建立 Qt compositor 骨架和自动化验证；
- 解耦 cutefish-core 包依赖；
- 冻结 X11/KWin 新功能，开发分支继续可启动；
- 产出功能等价矩阵和 X11/KWin 删除清单。

## 5. 可能遇到的问题

| 问题 | 说明与应对 |
|---|---|
| 删除 Xorg 后 SDDM 依赖冲突 | 产品不保留 SDDM；LoginShell 是发布门槛 |
| core 包重打包破坏现有安装 | 先在新构建环境验证 dpkg 依赖闭包 |
| 直接切换后安装链不可用 | `cutefish-installer` 验收作为 ISO 发布门槛 |
| CI 无 GPU | 使用 compositor `VirtualBackend` 验证协议和 QML；真机验证 `KmsBackend` |
| 上游 Qt 包仍有 X11 库 | 只承诺不运行 X server；库级清除延后到阶段 4 决策 |

## 6. 验收标准

- `cutefish-core` 新包依赖无 `kwin-wayland`/`xwayland`，无 X11 session；
- 产品构建不含 SDDM/X11 session；LoginShell 为发布门槛；
- Calamares 配置不再引用 xsession/Plymouth/sddm.service 的最终目标；
- `cutefish-compositor-core` 启动 Wayland server，`cutefish-shell` 和最小客户端可连接；
- CI 阻断新增 X11/KWin 符号；
- 不宣称阶段 0 为可发布镜像。

## 7. 交付物

1. 解耦后的 `cutefish-core` 包和构建验证。
2. `cutefish-compositor-core` / `cutefish-shell` 工程骨架。
3. X11/KWin 符号扫描 CI。
4. 功能等价矩阵。
5. 无过渡期决策记录。
