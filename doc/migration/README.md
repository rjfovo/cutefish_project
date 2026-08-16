# CutefishOS Wayland 迁移实时进度

> 本文档目录是迁移执行期间唯一实时进度记录区。方案文档仍以 `doc/future/` 为准且只读。
> 状态取值只能为：`未开始`、`进行中`、`阻塞`、`验收通过`。
> 所有阻塞项必须指向 `conflicts.md` 中的冲突编号。

## 总进度

- 当前阶段：stage-0（进行中）
- 当前任务：已完成新构建体系、compositor core/shell/installer 工程骨架、core 包边界迁移副本；
  剩余 core 服务迁移与 stage-0 全量验收
- 并行工作流：cutefish-installer 专用化（进行中，当前为 UI/工程骨架，磁盘 Job 禁止执行）
- 阻塞项：无 stage-0 级阻塞；stage-1 KMS Spike 的真机复验等待硬件批准（CONFLICT-006）

## 当前状态摘要

| 工作流 | 状态 | 说明 |
|---|---|---|
| stage-0 Wayland-only baseline | 进行中 | 构建系统、双 socket VirtualBackend、Qt shell 连接、core 包边界已通过初步验证 |
| stage-1 compositor/shell 闭环 | 未开始 | 待 stage-0 验收通过后进入 |
| stage-2 login/lock | 未开始 | 待 stage-1 验收通过后进入 |
| stage-3 boot/shutdown | 未开始 | 待 stage-2 验收通过后进入 |
| stage-4 purge | 未开始 | 待 stage-3 验收通过后进入 |
| installer 专用化 | 进行中 | 固定流程 UI 骨架可用；磁盘 Job 未实现且禁止在当前环境执行 |

## 本次会话已完成并验证

1. `wayland-scripts/build_code.sh` 四种旧用法、根目录覆盖、分类输出、源码副本 dpkg 构建、
   SOURCE_DATE_EPOCH、日志和 clean 边界均验证通过。
2. `cutefish-compositor-core` VirtualBackend + 双 Wayland socket + cutefish_core_v1 +
   minimal xdg-shell；协议冒烟测试通过；Qt `cutefish-shell` 以 Wayland QPA 连接通过；
   `--kms` 安全拒绝通过。
3. `cutefish-shell` Boot/Login/Session/Lock/Shutdown QML 骨架；offscreen QML 测试通过。
4. `cutefish-installer` 固定流程 UI 骨架；危险 Job 安全拒绝；offscreen QML 测试通过。
5. `cutefish/wayland-code/core` Wayland-only 包边界迁移副本构建为 `cutefish-core_0.9.0_amd64.deb`；
   无 X11 session 文件，无 kwin-wayland/xwayland/SDDM 依赖。
6. X11/KWin 源码符号扫描 0 命中；新包二进制直接依赖/符号扫描通过。
7. `wayland-scripts/iso/` 新 target/live 包清单和移除清单通过自检。

## 目录边界（执行层）

- 新源码：`cutefish/wayland-code/`
- 新构建脚本：`wayland-scripts/`
- 构建产物：`cutefish/output/build|packages|symbols|logs|cache/`
- 进度记录：`doc/migration/`
- 冻结只读：`cutefish/code/`、`script/`、`build_iso/`、`app/`、`doc/future/`

## 下一步

1. 继续 `core` 迁移副本：session、settings-daemon、powerman、notificationd、gmenuproxy、
   chotkeys 等按阶段改成 Wayland 原生/停用形式，并重新验证 cutefish-core 包。
2. 将 `wayland_protocol_smoke` 纳入包构建后 CI 测试路径（当前通过独立 CMake/CTest 执行）。
3. 完成 `wayland-scripts/iso/` 到实际 ISO 构建脚本的等价实现。
4. stage-0 全部验收项通过后，记录验收通过，再进入 stage-1。

## 未完成事项

- KMS/Libinput/Logind 真实硬件路径未实现，也未在任何环境执行 KMS 测试（见测试服务器边界）。
- 安装器磁盘 Job 未实现；只允许在专用硬件验证环境执行，禁止在测试服务器执行。
- 现有模块迁移副本未全部完成：session/settings-daemon/powerman/notificationd/gmenuproxy/
  chotkeys/desktop/dock/statusbar/launcher/filemanager/fishui/qt-plugins/settings/
  screenshot/screenlocker/libcutefish/screen 仍为后续任务。
- 测试服务器 192.168.118.132 尚未使用；当前所有验证均在本机隔离 /tmp 和 offscreen 环境完成。
