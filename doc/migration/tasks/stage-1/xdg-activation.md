# stage-1 任务：xdg_activation_v1

- 状态：进行中
- 方案依据：最小协议集包含 `xdg_activation_v1`，用于启动器和任务栏激活。

## 实际修改

- 生成 staging `xdg-activation-v1.xml` 协议代码。
- 新增 `wayland/xdg_activation.h/cpp`：
  - 全局 xdg_activation_v1；
  - activation token 生命周期、app_id/surface/serial、commit 后 `done` token；
  - `activate` 查找目标窗口并激活，发送 activated configure。
- 测试：`wayland_protocol_smoke` 创建 token、接收 done、activate 后收到
  activated configure；PASS。

## 验收状态

- 基础 token/activate 路径：通过。
- 未验收：serial/seat 校验策略、不同 client 间 token 传递、启动器真实场景。

## 模块迁移元数据

- 原项目路径：无（新增协议实现）
- 迁移后路径：`cutefish/wayland-code/cutefish-compositor-core/wayland/xdg_activation.*`
- 迁移前依赖：不适用
- 迁移后依赖：wayland-protocols staging xdg-activation-v1
- 架构变化：token 管理 + Workspace 激活
- 对外接口变化：xdg_activation_v1
- 功能差异/裁剪：未校验 seat/serial 时效
- 测试与验收：token→activate→configure 测试通过
