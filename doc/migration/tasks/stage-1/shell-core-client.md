# stage-1 任务：Shell 私有协议客户端与 com.cutefish.Display

- 状态：进行中
- 方案依据：`cutefish-dedicated-compositor-architecture.md` §8；`development-key-points.md` §6

## 实际修改

- `cutefish-shell` 增加协议 XML 副本和 wayland-client 生成代码。
- 新增 `ShellCoreClient`：
  - 连接 core shell socket（`CUTEFISH_SHELL_SOCKET`）；
  - 绑定 `cutefish_core_v1` v2；
  - 处理 lifecycle/output/window 事件；
  - 请求输出/窗口、SetOutputConfig/Activate/Close。
- 新增 `DisplayService`：
  - 用户会话总线注册 `com.cutefish.Display`；
  - `GetOutputs/SetMode/SetScale/SetTransform`；
  - 输出变化信号。
- 新增 `--display-service-test`。

## 测试结果

- 本机 dbus-run-session 测试：
  - core VirtualBackend 启动；
  - shell 连接 `CUTEFISH_SHELL_SOCKET`；
  - `gdbus call com.cutefish.Display.GetOutputs` 返回 VIRTUAL-1 1920x1080。
- offscreen QML 测试：PASS。

## 验收状态

- Shell → core 输出管理通路：基础通过。
- 未验收：设置应用真实 UI 联调、热插拔信号、SetMode/SetScale 端到端验证。

## 模块迁移元数据

- 原项目路径：无（新增 Shell 客户端）
- 迁移后路径：`cutefish/wayland-code/cutefish-shell/{shellcoreclient,displayservice}`
- 迁移前依赖：不适用
- 迁移后依赖：Qt Core/DBus、wayland-client
- 架构变化：Shell 经 shell socket 获取窗口/输出模型；普通应用不访问
- 对外接口变化：`com.cutefish.Display`
- 功能差异/裁剪：设置 UI 未对接
- 测试与验收：dbus-run-session GetOutputs 通过
