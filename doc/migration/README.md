# CutefishOS Wayland 迁移实时进度

> 本文档目录是迁移执行期间唯一实时进度记录区。方案文档仍以 `doc/future/` 为准且只读。
> 状态取值只能为：`未开始`、`进行中`、`阻塞`、`验收通过`。
> 所有阻塞项必须指向 `conflicts.md` 中的冲突编号。

## 总进度

- 当前阶段：stage-1（进行中）
- 当前任务：KmsBackend/LibinputBackend/LogindSession 工程实现与测试服务器安全探针
- 阻塞项：CONFLICT-006 部分解决；VMware 虚拟 GPU 探针通过，AMD/Intel 物理 GPU
  模式设置/上屏复验仍待设备或明确放宽策略

## 当前状态摘要

| 工作流 | 状态 | 说明 |
|---|---|---|
| stage-0 Wayland-only baseline | 验收通过 | 见 `tasks/stage-0/*.md` |
| stage-1 compositor/shell 闭环 | 进行中 | KmsBackend/Libinput/Logind 开发中；物理 GPU 复验待 CONFLICT-006 |
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

1. 完成 KmsBackend/LibinputBackend/LogindSession 编码与本地编译。
2. 在测试服务器 `/tmp/<test-dir>` 下运行非破坏性 KMS/GBM/libinput 能力探针并清理。
3. 继续实现最小协议集、WM 模型、最终合成与 Shell 窗口化。
4. 物理 AMD/Intel GPU 模式设置/上屏复验仍需专用设备或明确放宽策略（CONFLICT-006）。
5. stage-1 完成前不得发布产品镜像；无 X11/KWin/SDDM/Calamares 运行链回退。

## 未完成事项

- KMS/Libinput/Logind 真实硬件路径未实现，也未在任何环境执行 KMS 测试（见测试服务器边界）。
- 安装器磁盘 Job 未实现；只允许在专用硬件验证环境执行，禁止在测试服务器执行。
- 现有模块迁移副本：desktop/dock/statusbar/launcher/filemanager/fishui/qt-plugins/
  settings/screenshot/screenlocker/libcutefish/screen 按 stage-1 计划推进。
- `wayland-scripts/iso/` 目前为包清单和检查器，完整 live/target 构建脚本待实现。
