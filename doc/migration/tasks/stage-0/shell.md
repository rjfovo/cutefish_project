# stage-0 任务：cutefish-shell 工程骨架

- 状态：进行中
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

- 本任务项：验收通过（工程骨架范围）。
