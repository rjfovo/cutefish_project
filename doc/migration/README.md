# CutefishOS Wayland 迁移实时进度

> 本文档目录是迁移执行期间唯一实时进度记录区。方案文档仍以 `doc/future/` 为准且只读。
> 状态取值只能为：`未开始`、`进行中`、`阻塞`、`验收通过`。
> 所有阻塞项必须指向 `conflicts.md` 中的冲突编号。

## 总进度

- 当前阶段：stage-0 已验收通过；stage-1 未开始，等待 CONFLICT-006 KMS 真机复验硬件批准
- 当前任务：stage-1 前置条件确认；安装器专用化继续并行推进
- 阻塞项：CONFLICT-006（stage-1 KMS Spike 需要 AMD/Intel 真机，测试服务器禁止 DRM/input 验证）

## 当前状态摘要

| 工作流 | 状态 | 说明 |
|---|---|---|
| stage-0 Wayland-only baseline | 验收通过 | 见 `tasks/stage-0/*.md` |
| stage-1 compositor/shell 闭环 | 未开始 | 待 KMS 真机复验决策（CONFLICT-006） |
| stage-2 login/lock | 未开始 | 待 stage-1 验收通过后进入 |
| stage-3 boot/shutdown | 未开始 | 待 stage-2 验收通过后进入 |
| stage-4 purge | 未开始 | 待 stage-3 验收通过后进入 |
| installer 专用化 | 进行中 | 固定流程 UI 骨架可用；磁盘 Job 禁止在当前环境执行 |

## 已完成并验证

1. `wayland-scripts/build_code.sh` 四种旧用法、根目录覆盖、分类输出、源码副本 dpkg 构建、
   SOURCE_DATE_EPOCH、日志和 clean 边界均验证通过。
2. `cutefish-compositor-core` VirtualBackend + 双 Wayland socket + cutefish_core_v1 +
   minimal xdg-shell；协议冒烟测试通过；Qt `cutefish-shell` 以 Wayland QPA 连接通过；
   `--kms` 安全拒绝通过。
3. `cutefish-shell` 五态 QML 骨架；offscreen QML 测试和 Wayland 连接测试通过。
4. `cutefish-installer` 固定流程 UI 骨架；危险 Job 安全拒绝；offscreen QML 测试通过。
5. `cutefish/wayland-code/core` Wayland-only 包边界迁移副本：
   session、settings-daemon、powerman、notificationd、polkit-agent、clipboard、
   cpufreq、screen-brightness、shutdown-ui；deb 无 kwin-wayland/xwayland/SDDM 依赖，
   无 X11 session 文件。
6. X11/KWin 源码符号扫描 0 命中；新包二进制直接依赖/符号扫描通过。
7. `wayland-scripts/iso/` 新 target/live 包清单和移除清单通过自检。

## 目录边界（执行层）

- 新源码：`cutefish/wayland-code/`
- 新构建脚本：`wayland-scripts/`
- 构建产物：`cutefish/output/build|packages|symbols|logs|cache/`
- 进度记录：`doc/migration/`
- 冻结只读：`cutefish/code/`、`script/`、`build_iso/`、`app/`、`doc/future/`

## 下一步

1. 等待并记录 CONFLICT-006 决策：批准专用 KMS 真机验证环境或明确替代验证策略。
2. 批准后进入 stage-1：KmsBackend、LibinputBackend、LogindSession、完整最小协议、
   WM/最终合成、Shell 窗口化回归。
3. 并行继续安装器：polkit helper、固定 Job 清单、专用硬件验证环境下的安装测试。
4. stage-1 完成前不得发布产品镜像；无 X11/KWin/SDDM/Calamares 运行链回退。

## 未完成事项

- KMS/Libinput/Logind 真实硬件路径未实现，也未在任何环境执行 KMS 测试（见测试服务器边界）。
- 安装器磁盘 Job 未实现；只允许在专用硬件验证环境执行，禁止在测试服务器执行。
- 现有模块迁移副本：desktop/dock/statusbar/launcher/filemanager/fishui/qt-plugins/
  settings/screenshot/screenlocker/libcutefish/screen 按 stage-1 计划推进。
- `wayland-scripts/iso/` 目前为包清单和检查器，完整 live/target 构建脚本待实现。
