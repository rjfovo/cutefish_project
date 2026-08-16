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
