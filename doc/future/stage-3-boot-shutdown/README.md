# 阶段 3：Boot/ShutdownShell 与 Plymouth 边界收敛

> 所属主计划：`../wayland-migration-master-plan.md`
> 架构依据：`../cutefish-dedicated-compositor-architecture.md`
> 目标：`cutefish-shell` 提供 BootShell 和 ShutdownShell；Plymouth 只保留在 initramfs 阶段，
> rootfs 后退出，不追求删除 Plymouth 包。

## 1. 需要修改的项目

| 项目/路径 | 修改内容 |
|---|---|
| `cutefish-shell` | 新增 BootShell、ShutdownShell |
| `cutefish-shell/qml` | 开机背景、启动进度、关机动画 |
| `cutefish-compositor-core` | 提供 rootfs 后的启动进度数据；定义并验证 Plymouth → core 的 DRM master 交接时序 |
| `cutefish-compositor-core/systemd` | core 服务提前启动；Shell 按状态拉起 |
| `script/create_iso/build_*` | Plymouth 配置缩小为 initramfs splash，不删除包 |
| `build_iso/script/patch_*` | Plymouth 相关补丁只保留 initramfs 阶段 |
| `cutefish/code/plymouth-theme` | 保留但缩小主题用途；不再作为 rootfs 后的开机/关机动画 |
| GRUB/内核参数 | 与 initramfs splash 和 BootShell 首帧保持一致 |

## 2. 依赖

```text
systemd
systemd-logind
libdrm
libgbm
initramfs-tools
Plymouth（仅 initramfs splash）
品牌背景与字体资源
```

## 3. 需要解决的问题

1. `cutefish-compositor-core` 只能在 rootfs 挂载、udev/DRM 就绪后启动。
2. initramfs、root 挂载前、fsck 阶段的显示由 initramfs splash/Plymouth 负责。
3. BootShell 只显示 rootfs 之后的 systemd 启动进度；不显示 initramfs/fsck 进度。
4. initramfs splash 到 BootShell 的首帧必须一致，避免黑屏/闪屏/文本回退。
5. 关机路径必须在 systemd 停止 Qt/core 依赖服务前进入 ShutdownShell。
6. 不追求“GRUB→BootShell 全程无空窗”，只承诺“initramfs splash → BootShell 无闪屏”。

## 4. 功能变化

| 功能 | 变化 |
|---|---|
| initramfs 阶段 | 保留 Plymouth/极简 splash |
| rootfs 之后开机动画 | `cutefish-shell` BootShell |
| 启动进度 | BootShell 显示 rootfs 之后的 systemd 进度 |
| 关机动画 | `cutefish-shell` ShutdownShell |
| 显示生命周期 | core 常驻；Shell 按状态切换 |
| Plymouth 用途 | 从全局开关机动画缩减为 initramfs splash |

## 5. 可能遇到的问题

| 问题 | 应对 |
|---|---|
| initramfs 到 BootShell 闪屏 | 使用同一背景资源；像素级对比后切换 |
| 早期 DRM 未就绪 | core 等待 udev/DRM；超时显示最小错误画面 |
| 启动进度数据不可靠 | 只接入稳定的 systemd D-Bus 属性 |
| 关机时序竞争 | ShutdownShell 在显示相关服务停止前进入 |
| Plymouth 与 core 同时持有 DRM | Plymouth 必须在 core 启动前完全退出 |
| 移除 Plymouth 的旧脚本依赖 | 清理 rootfs 阶段 Plymouth 调用，保留 initramfs 配置 |

## 6. 验收标准

- initramfs splash → BootShell → LoginShell → SessionShell 无黑屏/闪屏/文本回退；
- BootShell 不承诺显示 initramfs/fsck 阶段进度；
- 关机动画稳定；
- Plymouth 只运行在 initramfs，rootfs 后无 plymouthd；
- 系统无 SDDM、Xorg、XWayland、KWin。

## 7. 交付物

1. BootShell 和 ShutdownShell。
2. core/Shell 的 systemd 启动链。
3. Plymouth 边界收敛后的 ISO。
4. 冷启动、关机、重启测试报告。
