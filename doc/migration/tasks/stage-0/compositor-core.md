# stage-0 任务：cutefish-compositor-core 工程骨架

- 状态：验收通过（stage-0 工程骨架范围）
- 方案依据：`stage-0` + `cutefish-dedicated-compositor-architecture.md`
- 禁止：QtWaylandCompositor API、wlroots、eglfs_kms、KWin/KDE 桌面栈、X11/KWin 代码。

## 实际修改

- `cutefish/wayland-code/cutefish-compositor-core/`
- backend：`DisplayBackend`/`Buffer` 接口 + `VirtualBackend`
- input/session：`InputBackend`/`SessionBackend` 版本化接口边界
- lifecycle：Boot/Login/Session/Lock/Shutdown 状态机
- wayland：双 socket（apps 不注册 cutefish_core_v1，shell 注册）；
  minimal wl_compositor/wl_shm/wl_seat/wl_output/xdg-shell；
  vendored `protocol/cutefish-core-v1.xml`
- data：`cutefish-compositor-core.service` 草案
- tests：`wayland_protocol_smoke`（双 socket 隔离、core 协议请求、KMS 拒绝由集成脚本覆盖）

## 执行命令

```text
cmake -S cutefish/wayland-code/cutefish-compositor-core -B cutefish/output/build/cutefish-compositor-core/cmake ...
cmake --build ... && ctest --test-dir ... --output-on-failure
./wayland-scripts/build_code.sh cutefish-compositor-core
./wayland-scripts/test_stage0_integration.sh
```

## 测试结果

- CTest `wayland_protocol_smoke`：PASS（0.34s）。
- 双 socket：shell socket 可见 `cutefish_core_v1` 与 xdg_wm_base；apps socket 可见基础协议但不可见 `cutefish_core_v1`。
- Qt `cutefish-shell --boot --connection-test` 以 Wayland QPA 连接 apps socket：退出码 0。
- `--kms`：退出码 2，不打开 DRM/input 设备。
- 干净 SIGTERM 退出：退出码 0。

## 验收状态

- stage-0 工程骨架：验收通过。
- KMS/Libinput/Logind 真机路径未实现，属于 stage-1，当前被 CONFLICT-006 阻塞。

## 模块迁移元数据

- 原项目路径：无（新增项目）
- 迁移后路径：`cutefish/wayland-code/cutefish-compositor-core`
- 迁移前依赖：不适用
- 迁移后依赖：Qt Core/Gui/DBus、libwayland-server、libdrm、gbm、libinput、libudev、libxkbcommon、wayland-protocols
- 架构变化：自研 core，禁止 QtWaylandCompositor/wlroots/eglfs_kms/KWin
- 对外接口变化：双 Wayland socket；shell socket 暴露 `cutefish_core_v1`，apps socket 不暴露
- 功能差异/裁剪：首版无手写笔/手势；KMS 安全门控
- 测试与验收：CTest、集成测试通过；stage-0 验收通过
