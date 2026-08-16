# 并行工作流任务：cutefish-installer 专用化

- 状态：进行中（stage-0 骨架通过，整体安装器未验收）
- 方案依据：`installer-simplification.md`；并行执行。
- 安全边界：当前环境禁止执行分区/格式化/grub-install/efivarfs 写入；测试服务器禁止磁盘 Job。

## 实际修改

- `cutefish/wayland-code/cutefish-installer/`
- 固定流程 UI：Welcome/Disk/Partition/Timezone/User/Summary/Progress/Finish。
- `InstallerBackend`：步骤状态机；`dangerousJobsAllowed=false`；`beginInstall()` 安全拒绝。
- `config/install.conf`：UEFI-only、无加密、无 LVM/RAID、ESP 300MiB 等固定策略。
- debian 打包草案。

## 执行命令

```text
ctest --test-dir cutefish/output/build/cutefish-installer/cmake --output-on-failure
./wayland-scripts/build_code.sh cutefish-installer
```

## 测试结果

- offscreen 固定流程 QML 冒烟：PASS。
- 危险 Job 调用：安全拒绝并记录日志。
- 未执行任何分区/格式化/引导安装命令。

## 验收状态

- UI/工程骨架：通过。
- 安装器整体验收：未验收（磁盘 Job、polkit helper、live ISO 切换未实现）。

## 模块迁移元数据

- 原项目路径：无（新增项目，替代 `cutefish/code/calamares` 与 `calamares-settings`）
- 迁移后路径：`cutefish/wayland-code/cutefish-installer`
- 迁移前依赖：Calamares/KPMcore/Python 模块体系
- 迁移后依赖：Qt Core/Gui/Quick/QuickControls2；磁盘 Job 待专用硬件验证
- 架构变化：固定流程、无插件、无 Python、polkit helper 规划中
- 对外接口变化：固定流程 UI；危险 Job 在 stage-0 安全拒绝
- 功能差异/裁剪：首版 UEFI-only；无 LVM/RAID/加密/BIOS
- 测试与验收：offscreen QML 通过；磁盘 Job 未验收
