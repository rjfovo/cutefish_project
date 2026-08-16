# stage-1 任务：WM 窗口模型与 xdg-toplevel 状态

- 状态：进行中
- 方案依据：`stage-1-qt-compositor-shell/README.md`、`cutefish-dedicated-compositor-architecture.md` §6/§10

## 实际修改

- 新增 `wm/window.h/cpp`：surface/xdg_surface/xdg_toplevel 关联、title/app_id、
  requested size、Maximized/Fullscreen/Minimized/Normal 状态、激活状态、configure serial。
- 新增 `wm/workspace.h/cpp`：窗口创建/销毁、surface/toplevel 查找、活动窗口、z-order 基础模型。
- `wayland/xdg_shell.cpp`：
  - xdg_surface 持有 surface；
  - get_toplevel 创建 Workspace Window 并发送初始 activated configure；
  - set_title/set_app_id 写入窗口模型；
  - maximize/unmaximize/fullscreen/unfullscreen/minimize 状态迁移并发送带 states 的 configure；
  - toplevel destroy/client 断开时销毁 Workspace Window。
- `tests/wayland_protocol_smoke.cpp`：扩展 xdg-shell 客户端冒烟，验证初始 activated configure、
  maximized configure states，以及 xdg_positioner → xdg_popup 初始 configure 几何。

## 测试结果

- `ctest --test-dir cutefish/output/build/cutefish-compositor-core/cmake --output-on-failure`：PASS。
- 测试覆盖：双 socket 隔离、cutefish_core_v1、xdg_toplevel 创建/configure/最大化状态、
  xdg_positioner/xdg_popup 初始 configure 几何。

## 验收状态

- 未验收。待完成：move/resize 交互、焦点/z-order 策略、最小化恢复、Qt Quick
  真实菜单/ComboBox/ToolTip 回归、与 Shell 私有协议窗口模型对接。

## 追加：cutefish_core_v1 窗口模型事件

- 协议版本升级到 2；新增 `get_windows` 请求和 `window`、`window_destroyed`、
  `window_state_changed` 事件（since=2，保持 v1 兼容）。
- Workspace/Window 增加窗口 id、添加/删除/状态变更信号。
- Server 将窗口模型广播到受信 Shell 的 `cutefish_core_v1` 资源。
- 协议冒烟测试覆盖 v2 窗口事件：PASS。

## 模块迁移元数据

- 原项目路径：无（新 WM 模型）
- 迁移后路径：`cutefish/wayland-code/cutefish-compositor-core/{wm,wayland}`
- 迁移前依赖：不适用
- 迁移后依赖：libwayland-server、wayland-protocols
- 架构变化：Window/Workspace 模型；xdg toplevel/popup/positioner
- 对外接口变化：`cutefish_core_v1` v2 窗口事件
- 功能差异/裁剪：move/resize 交互未完成
- 测试与验收：协议冒烟测试通过；交互未验收
