# CutefishOS Wayland 统一合成器路线

> 最后更新：2026-08-16
> 目标：用单一显示程序贯穿“开机 → 登录 → 桌面 → 关机”，为后续全面迁移
> Wayland 做准备。当前 X11/Plymouth/SDDM 栈保留为 fallback，不再扩展。

## 1. 目标架构

```text
GRUB
  ↓ 只负责选择和加载内核，视觉与下一帧相同
内核 / initramfs
  ↓ 不直接管理最终 UI
cutefish-compositor 启动
  ├── boot shell：品牌背景 + 真实启动进度
  ├── login shell：用户列表 / 密码 / 会话选择
  ├── session shell：桌面 / dock / 应用窗口
  ├── lock shell：锁屏
  └── shutdown shell：关机动画
系统服务全部在 compositor 之外并行初始化
```

核心原则：

```text
一个进程，持续拥有显示输出。
登录界面和桌面只是 compositor 内的不同 surface/模式。
没有 Xorg、没有独立 DM、没有 Plymouth 与 X 的 VT 交接。
```

## 2. 组件划分

建议新建独立模块：

```text
cutefish-compositor/
├── core/                 # QtWaylandCompositor + KWayland 封装
├── shell/                # boot/login/lock/session shell
├── auth/                 # PAM / logind / autologin 桥接
├── systemd/              # 启动与生命周期单元
├── qml/                  # 各阶段界面
└── tools/                # 诊断与配置工具
```

### 2.1 core

职责：

- 枚举 DRM 输出，管理分辨率/缩放/热插拔；
- 创建 Wayland 显示；
- 管理输入设备、焦点、seat；
- 提供 XWayland 兼容层给尚未迁移的 X11 应用；
- 管理 shell surface 层级：
  - boot splash
  - login
  - lock
  - desktop

### 2.2 shell

所有阶段共用一套渲染树：

```text
BackgroundSurface
  └── boot-background.png
        ├── BootShell
        ├── LoginShell
        ├── LockShell
        └── SessionShell
```

切换时只做淡入淡出，不重建 renderer。

### 2.3 auth

- 登录验证走 PAM；
- 自动登录走早期配置；
- 会话注册走 systemd-logind；
- 锁屏、挂起、关机/重启权限走 logind D-Bus。

## 3. 启动时序

### 3.1 最终目标

```text
GRUB
  → 内核加载 initramfs
  → udev / 存储 / 网络等服务在后台初始化
  → systemd 启动 cutefish-compositor.service
  → compositor 第一帧 = boot-background.png
  → compositor 进入 boot shell，显示进度
  → 系统基础服务就绪
  → compositor 切换到 login shell
  → 用户登录
  → compositor 切换到 session shell
  → 桌面
  → 关机时切换 shutdown shell
```

### 3.2 systemd 单元草案

```ini
[Unit]
Description=Cutefish Wayland Compositor
After=systemd-udev-settle.service systemd-logind.service
Conflicts=getty@tty1.service
Before=display-manager.target

[Service]
Type=simple
ExecStart=/usr/bin/cutefish-compositor
Restart=always
TTYPath=/dev/tty1
StandardInput=tty
StandardOutput=journal
```

## 4. 迁移阶段

### 阶段 0：冻结 X11 路线

- 当前 X11 版本作为 fallback 保留；
- 不再给 Plymouth/Xorg/SDDM 增加新机制；
- 只维护稳定性。

### 阶段 1：Wayland Session 验证

先不碰 boot/login，只验证桌面能否跑在 kwin_wayland 上：

- 新增 `cutefish-wayland-session.desktop`；
- `cutefish-session --wayland` 启动 `kwin_wayland`；
- 依次验证：
  - desktop / dock / statusbar
  - launcher
  - filemanager
  - settings
  - terminal
  - 截图/录屏
  - 锁屏
- 审计并移除 X11 专属依赖。

这一步会暴露大量兼容问题，但不改启动架构，风险可控。

### 阶段 2：compositor 接管桌面

- 将 `kwin_wayland` 替换为 `cutefish-compositor`；
- 桌面组件作为 Wayland 客户端运行；
- XWayland 继续承载未迁移应用；
- 登录仍可先用 SDDM Wayland greeter。

### 阶段 3：登录/锁屏并入 compositor

- 实现 LoginShell；
- 接入 PAM / logind；
- 去掉 SDDM；
- 自动登录和手动登录同一条代码路径。

### 阶段 4：boot/shutdown shell

- 实现 BootShell 和 ShutdownShell；
- compositor 提前到 systemd 早期启动；
- 最终移除 Plymouth 依赖；
- 关机时 compositor 保持运行到最后。

## 5. 与当前 X11 方案的兼容策略

```text
cutefish-xsession.desktop  → 保留
cutefish-wayland-session.desktop  → 新增
cutefish-compositor.service  → 新增，默认关闭
```

通过 SDDM/配置切换：

```text
X11 fallback   : 当前实现
Wayland session: 阶段 1
Compositor     : 阶段 2+
```

## 6. 风险与关键决策

| 风险 | 应对 |
|---|---|
| Cutefish 应用仍有 X11 专属代码 | 阶段 1 建立兼容清单，逐个迁移 |
| 截图/录屏权限 | 使用 xdg-desktop-portal + screencopy |
| 全局快捷键 | 迁移到 KGlobalAccel / compositor 快捷键层 |
| 锁屏安全 | compositor 内建 LockShell，不依赖独立锁屏进程 |
| XWayland 性能 | 仅用于兼容旧应用，新应用全部 native Wayland |
| 多屏/HiDPI | core 模块从第一版就实现 Wayland fractional scaling |
| 硬件差异 | 保留 X11 fallback，直到真机验证通过 |

## 7. 何时停止使用 Plymouth

```text
阶段 1：Plymouth 继续使用
阶段 2：Plymouth 继续覆盖 compositor 启动前空窗
阶段 3：Plymouth 仍保留，但只负责 initramfs 到 compositor 的第一段
阶段 4：compositor 第一帧与 Plymouth 最终帧完全一致后，逐步移除
```

## 8. 建议第一刀

不要先写 compositor，先做：

```text
cutefish-wayland-session.desktop
kwin_wayland 启动 Cutefish 桌面
兼容性审计清单
```

只有桌面组件全部能在 Wayland 正常工作时，统一合成器才有价值。
