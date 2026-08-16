# stage-0 任务：cutefish-shell 工程骨架

- 状态：验收通过（工程骨架范围）
- 方案依据：`stage-0` + `cutefish-dedicated-compositor-architecture.md`

## 实际修改

- `cutefish/wayland-code/cutefish-shell/`
- `main.cpp` 支持 `--boot|--greeter|--session|--lock|--shutdown|--self-test|--connection-test`
- QML：BootShell/LoginShell/SessionShell/LockShell/ShutdownShell；
  SessionShell 含 Desktop/Dock/StatusBar/Launcher 骨架
- debian 打包草案

## 执行命令

```text
ctest --test-dir cutefish/output/build/cutefish-shell/cmake --output-on-failure
./wayland-scripts/build_code.sh cutefish-shell
XDG_RUNTIME_DIR=<tmp> WAYLAND_DISPLAY=<apps-socket> QT_QPA_PLATFORM=wayland \
  cutefish-shell --boot --connection-test
```

## 测试结果

- offscreen QML 冒烟：PASS。
- Wayland QPA 连接 compositor-core：PASS（无窗口连接测试模式）。
- 全窗口渲染在 stage-1 合成/缓冲释放完成后继续验证；当前软件 backingstore 会等待缓冲释放，
  属已知 stage-1 工作项，不是本任务骨架阻塞。

## 验收状态

- stage-0 工程骨架：验收通过。

## 模块迁移元数据

- 原项目路径：无（新增项目）
- 迁移后路径：`cutefish/wayland-code/cutefish-shell`
- 迁移前依赖：不适用
- 迁移后依赖：Qt Core/Gui/Quick/QuickControls2/DBus、wayland-client
- 架构变化：可信 Qt Quick 客户端；Boot/Login/Session/Lock/Shutdown 五态
- 对外接口变化：`ShellCoreClient` 连接 shell socket；`com.cutefish.Display` 会话 D-Bus
- 功能差异/裁剪：当前为骨架，Desktop/Dock/StatusBar/Launcher 为占位
- 测试与验收：offscreen QML、Wayland 连接、D-Bus GetOutputs 通过
