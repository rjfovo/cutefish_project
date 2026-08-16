# stage-0 任务：cutefish-core Wayland-only 包边界迁移副本

- 状态：进行中
- 方案依据：`stage-0`；目录映射 `cutefish/code/core` → `cutefish/wayland-code/core`。
- 旧目录只读；禁止复制后原地改旧目录。

## 实际修改

- 新 `wayland-scripts/migrate-core-baseline.sh`：按显式 allow-list 生成迁移副本并记录源 commit。
- 迁移副本当前包含：clipboard、cpufreq、polkit-agent、screen-brightness、shutdown-ui。
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

- `cutefish-core_0.9.0_amd64.deb` 构建成功。
- Depends 无 kwin-wayland/xwayland/SDDM；内容无 `/usr/share/xsessions`。
- 新包只包含 5 个 Wayland-clean 组件；session/settings-daemon/powerman/notificationd 等
  服务迁移副本尚未完成，因此该任务不标记整体验收通过。

## 验收状态

- 包依赖解耦边界：通过。
- cutefish-core 全量服务等价迁移：未验收。
