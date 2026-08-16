# 模块迁移文档：session

## 1. 路径

- 原项目路径：`cutefish/code/core/session`（只读审计参考）
- 迁移后路径：`cutefish/wayland-code/core/session`

## 2. 依赖变化

- 迁移前：Qt Core/Gui/Widgets/Quick/DBus、KF6WindowSystem、QtCorePrivate/QtGuiPrivate、xcb、xcb-ewmh；安装 xsession 文件；启动 KWin/X11 桌面进程。
- 迁移后：Qt Core/DBus；不再链接 GUI/Widgets/Quick、KF6WindowSystem、xcb、xcb-ewmh；不安装 xsession。

## 3. 架构变化

删除 QAbstractNativeEventFilter/X11 WM 检测/KWin 启动逻辑；改为 Wayland-only 会话服务。职责收敛为环境初始化、语言/代理、用户 D-Bus 会话、用户 daemon 启动和 logind 电源操作。桌面/Dock/顶栏/启动器生命周期归 compositor core ShellManager。

## 4. 对外接口变化

保留 D-Bus 接口 com.cutefish.Session 的 logout/reboot/powerOff/suspend/startDesktopProcess/updateNetworkProxy/launch。launch 不再隐式设置 xcb 平台；默认 Qt 平台改为 wayland。startDesktopProcess 记录由 cutefish-shell 处理，不再拉起独立 desktop/dock/statusbar/launcher 进程。

## 5. 功能差异与首版裁剪项

首版裁剪：无 X11 splash；无并发多用户会话；用户切换定义为注销后回 LoginShell。电源动作经 loginctl/systemctl，执行需要 polkit 授权。

## 6. 测试与验收状态

编译通过；dbus-run-session + CUTEFISH_SESSION_TEST=1 冒烟通过；stage-0 包边界验收通过。完整登录/注销链待 stage-2。

## 7. 代码注释与安全边界

- 源码文件头注明 Wayland-only 会话服务职责。
- `power.cpp` 注释说明所有电源动作最终经 logind/systemd，不在会话进程内直接操作内核。
- `processmanager.cpp` 注释说明不再启动窗口管理器；daemon 清单可审计。
