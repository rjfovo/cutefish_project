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
