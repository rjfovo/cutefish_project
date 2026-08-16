# stage-1 任务：wl_seat 键盘/指针与 libinput 事件链路

- 状态：进行中
- 方案依据：`cutefish-dedicated-compositor-architecture.md` §6/§9；`development-key-points.md` §3.2

## 实际修改

- 新增 `wayland/seat.h/cpp`：
  - wl_seat v5 global，pointer/keyboard/touch resource 生命周期；
  - xkbcommon keymap 文本 + memfd，向 wl_keyboard 发送 xkb keymap；
  - 焦点 surface 切换时发送 pointer/keyboard enter/leave；
  - keyboard enter 后发送 modifiers；
  - pointer motion/button、keyboard key 事件发送。
- `wayland/server.cpp`：
  - 两个 Wayland display 都注册同一 Seat；
  - Workspace activeWindowChanged 联动 Seat focus；
  - run loop 将 libinput fd 纳入 poll，并把 LIBINPUT_EVENT_KEYBOARD_KEY、
    POINTER_MOTION、POINTER_BUTTON 翻译到 Seat。
- `main.cpp`：默认初始化 LibinputBackend 并交给 WaylandServer。
- `tests/wayland_protocol_smoke.cpp`：扩展验证 seat keymap、pointer enter、
  keyboard enter、modifiers。

## 测试结果

- `ctest --test-dir cutefish/output/build/cutefish-compositor-core/cmake --output-on-failure`：PASS。
- Qt shell Wayland 连接集成测试：PASS。
- 尚未在测试服务器执行真实键盘/鼠标事件注入；当前只做 libinput 能力探针。

## 验收状态

- 未验收。待完成：pointer axis/frame、touch、重复率、xkb layout 切换、
  真实输入设备端到端回归。

## 追加：pointer axis/touch 与键盘重复率

- Seat 增加 pointer axis/source/discrete/stop/frame、touch down/motion/up/frame/cancel、
  keyboard repeat_info。
- libinput 事件翻译增加 POINTER_AXIS、TOUCH_DOWN/MOTION/UP/FRAME/CANCEL。

## 模块迁移元数据

- 原项目路径：无（新增 Seat）
- 迁移后路径：`cutefish/wayland-code/cutefish-compositor-core/wayland/seat.*`
- 迁移前依赖：不适用
- 迁移后依赖：libwayland-server、libxkbcommon、libinput
- 架构变化：单一 Seat 跨双 socket；焦点由 Workspace 联动
- 对外接口变化：wl_seat v5、keymap、enter/leave、axis/touch/repeat
- 功能差异/裁剪：轴事件仅垂直滚轮；触摸无形状/方向
- 测试与验收：keymap/enter/modifiers 协议测试通过；真实输入端到端未验收
