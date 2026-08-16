# Stage 0~4 状态表

状态只能使用：未开始、进行中、阻塞、验收通过。阻塞必须指向 conflicts.md 编号。

| Stage | 状态 | 开始时间 | 验收状态 | 阻塞项 | 备注 |
|---|---|---|---|---|---|
| stage-0-wayland-only-baseline | 进行中 | 2026-08-16 | 未验收 | 无 | 工程基线已验证；core 服务迁移副本未全部完成 |
| stage-1-qt-compositor-shell | 未开始 | - | 未验收 | CONFLICT-006（KMS 真机复验等待硬件批准） | 未进入 |
| stage-2-login-lock | 未开始 | - | 未验收 | 无 | 未进入 |
| stage-3-boot-shutdown | 未开始 | - | 未验收 | 无 | 未进入 |
| stage-4-purge | 未开始 | - | 未验收 | 无 | 未进入 |
| installer 专用化（并行） | 进行中 | 2026-08-16 | 未验收 | 无 | UI/工程骨架可用；磁盘 Job 禁止在测试服务器执行 |
