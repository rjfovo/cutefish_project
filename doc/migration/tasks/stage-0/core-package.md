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

## 模块迁移元数据

- 原项目路径：`cutefish/code/core`
- 迁移后路径：`cutefish/wayland-code/core`
- 迁移前依赖：X11/XCB/KF6WindowSystem/KWin/SDDM helper 相关
- 迁移后依赖：Qt Core/Gui/Widgets/Quick/DBus/Xml、polkit-qt6、KF6IdleTime；无 kwin/xwayland/sddm
- 架构变化：Wayland-only 包边界，X11 组件不进入构建清单
- 对外接口变化：不安装 xsession；x-session-manager alternative 不注册
- 功能差异/裁剪：cupdatecursor/xembed-sni-proxy/sddm-helper/X11 session 移除
- 测试与验收：cutefish-core_0.9.0 deb 构建与依赖扫描通过
