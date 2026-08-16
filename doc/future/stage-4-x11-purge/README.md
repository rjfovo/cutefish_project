# 阶段 4：X11/KWin 彻底清除与冻结

> 所属主计划：`../wayland-migration-master-plan.md`
> 架构依据：`../cutefish-dedicated-compositor-architecture.md`
> 目标：Cutefish 自身源码、构建系统、软件包和运行系统中不再存在 X11/KWin 实现。

## 1. 需要修改的项目

| 项目/路径 | 修改内容 |
|---|---|
| `core/session` | 删除 xcb/EWMH、KWin 启动、X11 初始化遗留 |
| `core/chotkeys` | 删除 XGrabKey 实现 |
| `core/gmenuproxy` | 删除 xcb 属性代码 |
| `core/settings-daemon` | 删除 `mouse/x11*`、`touchpad/x11/`、`pkexec cutefish-sddm-helper --dpi` 调用 |
| `core/powerman` | 删除 X DPMS；明确 KF6IdleTime 去留 |
| `core/cupdatecursor` | 删除整个组件 |
| `core/xembed-sni-proxy` | 删除整个组件 |
| `core/shutdown-ui` | 删除或合并到 ShutdownShell |
| `core/clipboard` | 旧客户端删除；确认剪贴板已在 core 实现 |
| `core/notificationd` | 确认只保留 D-Bus 数据服务，通知 UI 在 shell |
| `libcutefish/screen` | 确认 KScreen 已由 `com.cutefish.Display` 替代 |
| `settings` | 确认光标解析不再依赖 Xcursor；使用 libwayland-cursor 或 core 接口 |
| 应用级 KF6 | 建立白名单，并对每个保留库做 X11 符号扫描 |
| `core/sddm-helper` | 删除整个组件 |
| `desktop` / `filemanager` | 删除 KX11Extras/`_NET_WM_*` |
| `dock` / `statusbar` / `launcher` | 删除 X11 窗口类型、strut、活动窗口代码 |
| `fishui` | 删除 X11 windowhelper/blur |
| `qt-plugins` | 删除 `x11integration.*` |
| `screenshot` | 删除 X11 抓屏路径 |
| `settings` | 删除 Xcursor/Xft/X11 查询 |
| `fantascene-dynamic-wallpaper` | 删除 X11 版本 |
| `yoyo-fantacy` | 从 ISO 包清单删除 |
| `kwin-plugins` | 删除整个模块 |
| CMake/debian | 删除 X11/XCB/KWin 查找与链接 |
| `script/create_iso`、`build_iso` | 清理 Xorg/XWayland/KWin 包、脚本、配置 |
| 应用层 | 清理 `DISPLAY`/`XAUTHORITY`/xcb 环境变量与日志 |
| `calamares` / `calamares-settings` | 从 ISO 删除；安装器为 `cutefish-installer`，不写入 SDDM/Plymouth/X11 session |

## 2. 依赖

### 2.1 删除的构建依赖

```text
libx11-dev
libxcb*-dev
libxcursor-dev
libxi-dev
libxtst-dev
libxfixes-dev
libxrender-dev
libxext-dev
xorg-server-dev / xorg-libinput / xorg-synaptics
kwin-dev / kwin-x11 / kwin-wayland / kwin-common
libkdecorations3-6
```

### 2.2 删除的运行时依赖

```text
xserver-xorg*
xinit
xterm
xauth
xrdb
xwayland
kwin-x11 / kwin-wayland
/usr/share/xsessions/*
```

### 2.3 Qt 上游 X11 库策略

原厂 Debian Qt：

```text
qt6-base-dev → libgl-dev → libglx-dev → libx11-dev
libqt6gui6 运行时依赖 libx11/libxcb
```

因此阶段 4 验收标准修正为：

- Cutefish 自身代码 0 X11/KWin 符号；
- Cutefish 构建依赖图中无直接 X11/XCB/KWin 开发包；
- 系统不安装、不运行 Xorg/XWayland/KWin；
- 接受 Qt 上游包引入的 libx11/libxcb 客户端库存在；
- 若要库级彻底删除，必须定制 Qt 并单独立项。

## 3. 需要解决的问题

1. 源码中仍有 X11/KWin 分支、宏、CMake 查找和日志。
2. 包依赖中可能仍有 X11 库被 `dpkg-shlibdeps` 自动写入。
3. ISO 构建脚本中仍有 Xorg、xterm、xinit、xauth、xrdb、yoyo-fantacy。
4. 系统中可能存在 `/usr/share/xsessions`、`DISPLAY`、X11 autostart。
5. 历史文档和脚本中仍有“X11 fallback”“KWin”旧策略描述。
6. Qt 包可选 X11 能力需要按 2.3 节策略处理。

## 4. 功能变化

本阶段不改变用户可见功能，只做清理。

## 5. 可能遇到的问题

| 问题 | 应对 |
|---|---|
| 删除代码导致编译错误 | 每个删除立即全量构建 |
| `dpkg-shlibdeps` 仍写入 libx11/libxcb | 检查 `ldd` 和包依赖报告 |
| 应用仍有隐藏 X11 调用 | 运行日志 + 符号扫描 + 真机回归 |
| Qt 包仍携带 X11 库 | 接受上游库存在，不运行 X server |
| 删除包后安装器/升级路径断裂 | 全新 ISO 和升级测试双验证 |
| 文档/脚本残留 X11/KWin 术语 | 全仓文档和脚本审计 |

## 6. 验收标准

- 源码扫描 X11/XCB/Xlib/KWin 符号为 0；
  扫描范围仅限 Cutefish 自研源码；生成代码、第三方子模块、文档、脚本单独分类；
  应用级 KF6 保留库单独建立白名单，并逐库扫描 X11 符号；
- 构建依赖图中无直接 X11/XCB/KWin 开发包；
- Cutefish 自身二进制无 libx11/libxcb 直接动态链接；
- 系统无 Xorg、XWayland、KWin、xinit、xterm、xauth、xrdb、X11 session；
- 功能回归全部通过；
- CI 进程白名单禁止 Xorg/XWayland/KWin。

## 7. 交付物

1. X11/KWin-free 源码主线。
2. X11/KWin-free 构建配置和 ISO。
3. 清除审计报告。
4. 符号和依赖扫描 CI。
5. 最终功能回归报告。
