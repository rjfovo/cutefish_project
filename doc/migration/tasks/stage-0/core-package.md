# stage-0 任务：cutefish-core Wayland-only 包边界迁移副本

- 状态：验收通过（stage-0 包边界范围）
- 方案依据：`stage-0`；目录映射 `cutefish/code/core` → `cutefish/wayland-code/core`。
- 旧目录只读；禁止复制后原地改旧目录。

## 实际修改

- 新 `wayland-scripts/migrate-core-baseline.sh`：按显式 allow-list 生成迁移副本并记录源 commit。
- 迁移副本当前包含：session、settings-daemon、powerman、notificationd、
  polkit-agent、clipboard、cpufreq、screen-brightness、shutdown-ui。
- session/settings-daemon/powerman/notificationd 为 Wayland 原生迁移实现；
  旧 X11 路径未进入副本。
- 新构建清单不包含：cupdatecursor、xembed-sni-proxy、sddm-helper、X11 session、
  chotkeys(X11)、gmenuproxy(X11)、settings-daemon(X11)、powerman(X DPMS)、
  session(X11/KWin)、notificationd 旧 UI。
- debian/control 移除 kwin-wayland/xwayland 及 X11/XCB/KWin 开发依赖；
  postinst/prerm 不再注册 x-session-manager。

## 执行命令

```text
./wayland-scripts/migrate-core-baseline.sh
./wayland-scripts/build_code.sh core
dpkg-deb -I cutefish/output/packages/core/cutefish-core_0.9.0_amd64.deb
dpkg-deb -c cutefish/output/packages/core/cutefish-core_0.9.0_amd64.deb
```

## 测试结果

- `cutefish-core_0.9.0_amd64.deb` 构建成功，包含 9 个 Wayland-clean 服务/工具。
- Depends 无 kwin-wayland/xwayland/SDDM；内容无 `/usr/share/xsessions`。
- chotkeys/gmenuproxy 的 Wayland 目标在 stage-1（core 输入链 / 用户会话 D-Bus
  Registrar），本阶段不把旧 X11 实现迁入新包。
- 旧 cupdatecursor/xembed-sni-proxy/sddm-helper 已排除出新构建清单。

## 验收状态

- stage-0 cutefish-core 包边界：验收通过。
- 后续服务等价性回归：随 stage-1 功能矩阵推进。
