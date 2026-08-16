# X11/KWin 删除与新目录策略清单

按任务定义，“删除对应实现”指：新代码/迁移副本中不存在、旧实现不进入新构建清单、
旧实现不进入 ISO 包清单；`cutefish/code/` 保持只读。

## 新构建清单排除项

- [x] cupdatecursor
- [x] xembed-sni-proxy
- [x] sddm-helper
- [x] X11 session 文件
- [x] kwin-wayland 依赖
- [x] xwayland 依赖
- [x] SDDM 产品运行链
- [x] Calamares/calamares-settings 产品运行链
- [x] kwin-plugins
- [x] sddm-theme 产品运行链
- [ ] 其余旧 X11 模块的迁移副本（进行中，按阶段推进）

## 旧 ISO 审计结果（只读审计）

`build_iso/` 与 `script/create_iso/` 中仍存在旧引用：
- `build_iso/script/live/cutefish_installer` 使用 sudo calamares；
- `build_iso/config/cutefish-installer.desktop` 指向 calamares；
- `build_iso/script/patch_plymouth_persistent.sh` 写入 SDDM/Plymouth 运行链；
- `build_iso/script/sddm-switch-display-server.sh` 切换 SDDM 显示服务器；
- `build_iso/config/sddm_autologin.conf` 引用旧 session；
- `script/create_iso/build_live_filesystem.sh` 安装 xserver/xinit/xterm 和旧组件。

按目录冻结规则，旧文件不修改；新 ISO 等价配置写入 `wayland-scripts/iso/`，
旧脚本只作为审计参考，不作为新构建目标。

## 新代码符号扫描

- 扫描脚本：`wayland-scripts/check_x11_symbols.sh`
- 当前结果：`cutefish/wayland-code/` 0 命中。
- 二进制直接依赖/符号扫描：`wayland-scripts/check_binary_symbols.sh`
- 当前结果：PASS（Qt 上游传递依赖按例外策略处理）。

## 待处理

- stage-1~4 按阶段推进各模块迁移副本后，本清单逐项关闭。
- 应用层 `app/` 默认冻结；如需源码级清理，先停止并写入 CONFLICT。
