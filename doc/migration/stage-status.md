# Stage 0~4 状态表

状态只能使用：未开始、进行中、阻塞、验收通过。阻塞必须指向 conflicts.md 编号。

| Stage | 状态 | 开始时间 | 验收状态 | 阻塞项 | 备注 |
|---|---|---|---|---|---|
| stage-0-wayland-only-baseline | 验收通过 | 2026-08-16 | 通过 | 无 | 工程基线、构建系统、core/shell/installer 骨架、core 包边界均验证通过 |
| stage-1-qt-compositor-shell | 进行中 | 2026-08-16 | 未验收 | CONFLICT-006（VMware 能力探针通过；AMD/Intel 物理 GPU 复验仍待设备） | Seat/输入链路、窗口模型、xdg-shell 状态路径推进中 |
| stage-2-login-lock | 未开始 | - | 未验收 | 无 | 未进入 |
| stage-3-boot-shutdown | 未开始 | - | 未验收 | 无 | 未进入 |
| stage-4-purge | 未开始 | - | 未验收 | 无 | 未进入 |
| installer 专用化（并行） | 进行中 | 2026-08-16 | 未验收 | 无 | UI/工程骨架通过；磁盘 Job 未实现且禁止在测试服务器执行 |
