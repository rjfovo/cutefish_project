# CutefishOS 安装器专用化精简方案

> 状态：V2.0，按专项评审修订，待批准
> 目标：不再把 Calamares 作为“通用、可扩展安装框架”使用，而是将其裁剪或替换为
> CutefishOS 的专用安装器。
> 最高准则：本模块同样必须遵守最高四原则：**专用、简洁、稳定、可替换**。
> 迁移衍生决策：无过渡期，Calamares 直接从产品删除。

## 1. 现状与问题

当前产品使用：

```text
cutefish/code/calamares
cutefish/code/calamares-settings
```

Calamares 是为多发行版设计的通用安装框架，存在以下不符合四原则的部分：

- 模块化加载器、Python 模块、插件机制；
- 大量发行版适配配置；
- 通用显示管理器、引导器、initramfs、plymouth 配置模块；
- 多语言、多键盘、多时区、多用户场景；
- 与 KDE KPMcore 和 KF6 模块耦合。

对 CutefishOS 内部版本而言，这些通用能力多数不需要，且会持续引入维护和兼容成本。

## 2. 最终目标

最终产品只保留一个固定安装流程：

```text
启动
  → 品牌欢迎页
  → 选择目标磁盘
  → 分区方式：默认自动分区，高级分区作为可选项
  → 时区选择
  → 创建用户
  → 确认摘要
  → 执行安装
  → 完成并重启
```

删除：

```text
插件/模块加载器
Python 安装模块（最终阶段）
第三方发行版适配
网络安装、软件包选择
复杂手动分区（高级分区有明确边界）
显示管理器选择
Plymouth 桌面阶段配置
SDDM/X11 session 配置
```

## 3. 目标模块清单

| 模块 | 是否保留 | 说明 |
|---|---|---|
| 品牌欢迎页 | 保留 | 静态品牌背景，不加载外部 QML 插件 |
| 磁盘选择 | 保留 | 固定磁盘选择，单盘优先 |
| 自动分区 | 保留，默认 | 固定分区策略，普通用户默认使用 |
| 高级分区 | 保留，可选 | 隐藏在高级选项；首版只允许选择已有分区并指定挂载点 |
| 时区选择 | 保留 | 提供常用时区搜索和地图/列表选择 |
| 用户创建 | 保留 | 用户名、密码、自动登录开关 |
| 摘要确认 | 保留 | 固定摘要 |
| 安装执行 | 保留 | 执行 Job 清单，见 §4.2 |
| 完成重启 | 保留 | 单一完成页 |
| 键盘/语言 | 默认预置 | 设置中心可改，不进入安装器必选流程 |
| 网络安装 | 删除 | 产品为本地安装 |
| 软件包选择 | 删除 | 固定包清单 |
| 加密安装 | 删除或后置 | 首版不启用 |
| LVM/RAID/resize/重建已有系统 | 删除 | 首版高级分区不提供 |
| displaymanager/services-systemd/plymouthcfg | 删除 | compositor 负责登录、开机和关机 |

## 4. 技术路线

### 4.1 最终推荐：`cutefish-installer` 专用应用

```text
cutefish/code/cutefish-installer
├── src/
│   ├── main.cpp
│   ├── installer.h/cpp
│   ├── diskops.h/cpp
│   ├── userops.h/cpp
│   ├── bootloader.h/cpp
│   ├── jobs.h/cpp
│   └── polkit_helper.cpp
├── qml/
│   ├── Main.qml
│   ├── Welcome.qml
│   ├── Disk.qml
│   ├── PartitionAdvanced.qml
│   ├── Timezone.qml
│   ├── User.qml
│   ├── Summary.qml
│   ├── Progress.qml
│   └── Finish.qml
└── config/
    └── install.conf
```

- UI 使用 Qt Quick + FishUI；
- 无插件、无 Python、无外部模块加载；
- 默认自动分区；
- 高级分区首版边界：**只允许选择已有分区并指定挂载点**；
- 不支持 resize、删除/重建已有系统、LVM/RAID/加密；
- 时区写入目标 rootfs，不修改 live 系统时区；
- 高权限操作通过独立 polkit helper；
- 安装目标为固定包清单和固定配置。

### 4.2 安装执行 Job 清单

所有步骤必须明确执行位置：`UI`、`helper` 或 `chroot`。

| # | Job | 执行位置 | 说明 |
|---|---|---|---|
| 1 | 挂载源 squashfs | helper | `/run/live/medium/live/filesystem.squashfs` 只读挂载 |
| 2 | 磁盘模式分支 | helper | 见下方“自动分区 / 高级分区”分支 |
| 3 | 挂载目标 root / ESP | helper | 创建 target 挂载点 |
| 4 | 复制 rootfs | helper | 先复制 rootfs，再做 bind mount；rsync 参数见下 |
| 5 | bind mount | helper | `/proc /sys /dev /run`；并挂载 efivarfs |
| 6 | 生成 fstab | helper | 按 UUID 写 `/etc/fstab` |
| 7 | machine-id | helper | 删除源 machine-id，生成新 `/etc/machine-id` 和 `/var/lib/dbus/machine-id` |
| 8 | random-seed | helper | 生成或清理 `/var/lib/systemd/random-seed` |
| 9 | hostname | helper | 写 `/etc/hostname` 和 `/etc/hosts` |
| 10 | 时区 | helper | 写目标 rootfs，不修改 live 时区 |
| 11 | locale/keyboard | helper | 写入目标默认 locale 和键盘配置 |
| 12 | 创建用户 | helper | 用户名、密码、默认组、sudo、自动登录配置 |
| 13 | 启用服务 | helper | 启用 core 系统单元；用户单元由包 postinst 负责 |
| 14 | grub-install | chroot | UEFI 固定参数，见 §4.2.3 |
| 15 | grub-mkconfig | chroot | 生成目标 grub.cfg |
| 16 | update-initramfs | chroot | 更新目标 initramfs |
| 17 | 清理构建残留 | helper | 见 §4.3 |
| 18 | 卸载并 sync | helper | 逆序卸载 bind mount、efivarfs、root、ESP、源 squashfs，最后 sync |

#### 4.2.1 自动分区 / 高级分区分支

```text
自动分区：
  1. 校验目标盘不是 live 启动介质；
  2. wipefs 整盘；
  3. 创建 GPT；
  4. 创建 ESP 和 root；
  5. 格式化 ESP 和 root。

高级分区：
  1. 校验目标盘不是 live 启动介质；
  2. 不 wipefs 整盘，不创建分区；
  3. 仅对用户选择的已有分区执行 wipefs + 格式化；
  4. 校验挂载点已分配；
  5. 校验目标分区未挂载且非 busy；
  6. 明确提示不会执行整盘清空。
```

#### 4.2.2 自动分区固定策略

```text
GPT
ESP：300 MiB，FAT32，挂载 /boot/efi，boot/esp 标志
root：剩余空间，ext4，挂载 /
无 swap
最低磁盘容量：安装 rootfs 大小 + ESP + 余量，构建时写入 install.conf
```

#### 4.2.3 GRUB 固定参数

```text
grub-install --target=x86_64-efi \
  --efi-directory=/boot/efi \
  --bootloader-id=cutefish \
  --recheck

grub-mkconfig -o /boot/grub/grub.cfg
```

- 默认不安装 removable fallback；
- 如后续需要内部恢复盘，再显式增加 `--removable` 并补充测试。

#### 4.2.4 rootfs 复制与 bind mount 顺序

必须先复制 rootfs，再 bind mount：

```text
rsync -aAXHAS --numeric-ids \
  --exclude=/proc/ \
  --exclude=/sys/ \
  --exclude=/dev/ \
  --exclude=/run/ \
  --exclude=/boot/efi/ \
  <source>/ <target>/
```

bind mount 补：

```text
mount --bind /proc <target>/proc
mount --bind /sys <target>/sys
mount --bind /dev <target>/dev
mount --bind /run <target>/run
mount -t efivarfs efivarfs <target>/sys/firmware/efi/efivars
```

efivarfs 异常：

- 优先尝试正常 `grub-install`；
- 若 NVRAM 不可写，必须降级为 removable 路径：
  `grub-install --no-nvram --removable --efi-directory=/boot/efi --bootloader-id=cutefish`；
- 明确提示用户这是降级路径；
- 若 removable 安装也失败，则安装失败，不得继续。

#### 4.2.5 hostname、DNS、服务启用和用户创建

hostname：

```text
默认值：hostname=cutefish，写入 install.conf；
echo cutefish > <target>/etc/hostname；
在 <target>/etc/hosts 写入 127.0.1.1 cutefish。
```

DNS（选择方案 B：NetworkManager 管理）：

```text
目标系统使用 NetworkManager 管理 DNS；
安装器创建 /etc/resolv.conf -> /run/NetworkManager/resolv.conf；
不保留 live 构建期的 resolv.conf；
target rootfs 不安装 systemd-resolved。
```

服务启用：

```text
系统单元：
  执行前确认 <target>/run/systemd/system 存在，并确保 bind mount 已完成；
  systemctl enable --root=<target> cutefish-compositor-core.service

用户单元：
  由软件包 postinst 提供；
  安装器只写自动登录/首启配置，不手工 enable 用户单元。
```

用户创建：

```text
useradd -R <target> 或 chroot useradd；
设置密码：echo '<user>:<password>' | chpasswd -R <target>；
默认组：adm/cdrom/sudo/dip/plugdev/bluetooth/lpadmin/sambashare；
写 Cutefish 自动登录配置（core auth 配置格式）。
```

#### 4.2.6 磁盘选择保护

磁盘选择模块必须：

```text
排除 live 启动介质所在块设备（/run/live/medium）；
显示型号、序列号、容量；
用户确认后才能进入分区/格式化；
目标盘 busy 时禁止继续。
```

### 4.3 构建残留清理清单

安装器复制 rootfs 后必须清理：

```text
/etc/apt/sources.list.d/cutefish.list      # 删除构建服务器地址
/etc/resolv.conf                            # 重建为 /run/NetworkManager/resolv.conf 符号链接
/etc/machine-id                             # 重新生成
/var/lib/dbus/machine-id                    # 重新生成
/usr/share/dbus-1/system.d/org.kde.kpmcore.helperinterface.conf  # 删除
```

### 4.4 磁盘后端边界

- 自动分区：使用 `sfdisk` 脚本化分区，不使用 sgdisk；
- 分区表重读：`partprobe` + `udevadm settle`；
- 高级分区首版：只允许选择已有分区并指定挂载点，不解析复杂磁盘拓扑；
- 高级分区只使用 parted 做只读信息展示和校验；
- 未来如需要完整图形分区器，再评估 libblockdev/libparted API，首版不做。

### 4.5 首版固件范围

首版只支持 UEFI：

```text
grub-efi-amd64
grub-efi-amd64-bin
grub-efi-amd64-signed
```

不保留 BIOS/`grub-pc` 分支；若未来增加 BIOS，必须单独增加包清单和
`grub-install --target=i386-pc` 测试项。

### 4.6 无过渡期：直接切换

- 不保留 Calamares 过渡版本；
- 不在产品中继续使用 Python 安装模块；
- `cutefish-installer` 是安装器唯一实现；
- 从切换版本开始，live ISO 直接安装并启动 `cutefish-installer`；
- Calamares 和 calamares-settings 从 ISO 和产品包清单中删除；
- Calamares 源码只保留在 git 历史中，不作为产品构建目标。

## 5. polkit helper 运行模型

- 定义 polkit action：

```text
com.cutefish.installer.partition
com.cutefish.installer.install
com.cutefish.installer.bootloader
```

- live 自动登录用户策略：

```text
首版：live 用户在图形会话中免密，但每个 helper 调用仍走 polkit 授权记录；
后续如需密码，改为输入固定 live 密码或管理员密码。
```

- UI 与 helper 协议：

```text
StartJob(jobId, params)
Progress(jobId, percent, message)
Log(jobId, severity, text)
Cancel(jobId)            # 仅在安全点生效
Finish(jobId, exitCode, error)
```

- wipefs 开始后 UI 禁用取消按钮；
- helper 超时、崩溃、退出必须能中止 UI 并显示可诊断日志。

## 6. live 与 target 包清单

### 6.1 target 固定包清单

```text
cutefish-compositor-core
cutefish-shell
cutefish-core（Wayland-only 拆包后：session/settings-daemon/powerman/notificationd/polkit-agent）
qt6-wayland
qt6-qpa-plugins
Cutefish 应用固定清单
```

target 不安装：

```text
cutefish-installer
calamares
calamares-settings
sddm
X11 session
```

target 对 Plymouth：

```text
Plymouth 仅保留 initramfs 所需包/主题；
不配置 rootfs 后的开机动画。
```

### 6.2 live 运行依赖

```text
cutefish-installer
polkitd / pkexec
cutefish-polkit-agent
squashfs-tools
rsync
sfdisk / parted（只读校验）
partprobe / udevadm
dosfstools
e2fsprogs
grub-efi-amd64 工具链
```

## 7. ISO 切换清单

切换 `cutefish-installer` 时必须修改：

```text
script/create_iso/build_live_filesystem.sh
  → 安装 cutefish-installer，删除 calamares/calamares-settings
  → 按 §6.2 完整安装 live 运行依赖（含 parted、sfdisk、partprobe 等）

build_iso/config/cutefish-installer.desktop
  → Exec=/usr/bin/cutefish-installer

build_iso/script/live/cutefish_installer
  → 删除 sudo calamares wrapper

script/build_code.sh
.gitmodules
script/download_codes.sh
  → 新增 cutefish-installer 仓库；移除 calamares、calamares-settings 构建条目

build_iso/package/cutefish
  → 新 deb 的复制和仓库重建

install rootfs 固定包清单
  → 不包含 cutefish-installer，只包含 target 包
```

## 8. 与 Wayland 迁移的关系

- 安装器不负责选择 X11/Wayland 会话；
- 安装器只安装 target 固定包清单；
- 不再写入：
  - `/usr/share/xsessions/cutefish-xsession.desktop`
  - `sddm.service`
  - Plymouth 桌面阶段配置
- 安装器写入：
  - `cutefish-compositor-core.service`
  - `cutefish-shell` 的 systemd 用户单元
  - Cutefish 自动登录/首启配置

## 9. 实施顺序

| 阶段 | 工作 |
|---|---|
| 0 | 停止构建 Calamares；从 ISO 和包清单删除 calamares/calamares-settings；新增 cutefish-installer 工程 |
| 1 | 实现 `cutefish-installer` MVP：欢迎、磁盘、自动/高级分区、时区、用户、进度、完成 |
| 2 | 接入 polkit helper、Job 清单、分区、格式化、rootfs 复制、引导安装 |
| 3 | live ISO 直接启动 `cutefish-installer`，不再经过 sudo calamares wrapper |
| 4 | 真机安装验证：安装后系统能启动到 `cutefish-compositor-core` 和 `cutefish-shell` |

## 10. 验收标准

- ISO 安装流程固定，无插件/模块加载器；
- 默认自动分区可用；
- 高级分区首版可选择已有分区并指定挂载点；
- 时区选择写入目标系统，不修改 live 时区；
- 安装器可在纯 Wayland 环境运行；
- 安装后系统只包含 target 固定包清单；
- machine-id、random-seed、hostname、locale、fstab、用户/自动登录、initramfs、grub 配置正确；
- 构建残留清理完成；
- 安装出的系统可启动到 LoginShell；
- 分区错误、磁盘忙、空间不足等异常有明确错误页；
- 安装器自身不依赖 KWin/KDE 桌面栈。

## 11. 风险与应对

| 风险 | 应对 |
|---|---|
| 分区/格式化由系统命令执行，错误处理复杂 | 固定自动分区策略；高级分区边界收窄；polkit helper 独立进程 |
| 引导安装依赖 grub 和固件 | 首版只支持 UEFI，BIOS 不承诺 |
| 直接切换导致安装器未完成前无可用 ISO | `cutefish-installer` 作为 ISO 发布门槛，验收前不发布该版本 |
| 构建残留导致目标系统异常 | §4.3 清理清单作为强制 Job |
| 安装器开发周期影响 ISO | 与显示迁移并行，不作为 Wayland 方案的前置阻塞 |
| FishUI 未完成 Wayland 解耦 | 安装器依赖 FishUI；FishUI X11 清理先于安装器验收 |
| 无过渡期与阶段 0 分支关系 | 开发分支在 cutefish-installer 可用前不发布 ISO；旧 X11 分支不得合并“停止构建 Calamares” |

## 12. 四原则检查

- **专用**：固定安装流程、固定包清单、固定硬件；
- **简洁**：无插件、无 Python、无发行版适配；
- **稳定**：固定 Job 清单、错误页、polkit helper；
- **可替换**：磁盘操作、用户创建、引导安装均抽象为独立接口。
