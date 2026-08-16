# 功能等价矩阵

状态规则：仅 `验收通过` 的功能可视为等价完成；其余为 `未开始` 或 `进行中`。
本矩阵随阶段推进更新，当前 stage-0 不承诺产品级功能等价。

| 功能域 | X11 基线功能 | Wayland/Qt 目标 | 状态 | 验证位置/说明 |
|---|---|---|---|---|
| 桌面 | 壁纸、图标、右键菜单、多屏、文件拖放、视频壁纸 | Shell Desktop QML | 未开始 | 骨架已建立 |
| 窗口管理 | 标题栏、移动/缩放/最小化/最大化/关闭、焦点、分组 | core WM + FishUI CSD | 进行中 | 仅 minimal xdg-shell 连接，无 WM 策略 |
| 面板 | 顶栏占位、应用菜单、托盘、网络/音量/电池、通知 | Shell StatusBar | 未开始 | 骨架已建立 |
| 启动器 | 显示/隐藏、搜索、启动、固定 | Shell Launcher + xdg-activation | 未开始 | 骨架已建立 |
| 会话 | 登录、注销、自动登录、锁屏、用户切换 | core lifecycle + Shell | 进行中 | 状态机骨架，无 PAM/logind |
| 输入 | 键盘、鼠标、触控板、触摸屏、快捷键、fcitx5/ibus | libinput + text-input-v3 + input-method-v2 | 未开始 | 接口边界已定义 |
| 显示 | 单/多屏、热插拔、旋转、缩放、DPMS | DisplayBackend + com.cutefish.Display | 进行中 | VirtualBackend 单输出；KMS 待 CONFLICT-006 |
| 截图录屏 | 全屏/区域/窗口、延时、剪贴板 | core 抓帧 + portal | 未开始 | 接口边界未实现 |
| 电源 | 息屏、待机、恢复、合盖、关机/重启 | core + logind | 未开始 | 接口边界已定义 |
| 托盘 | SNI 注册、菜单、状态 | 仅 SNI | 未开始 | xembed 已排除出新清单 |
| 全局菜单 | Qt/GTK DBusMenu | D-Bus Registrar + Shell 查询 | 未开始 | gmenuproxy 旧实现未迁入 |
| 剪贴板/主选择 | 关闭应用保留、中键粘贴 | core data-control + 缓存 | 未开始 | 协议未实现 |
| DnD | 跨应用拖放、拖放图标、drop/cancel | core drag surface | 未开始 | 协议未实现 |
| 通知 | 弹窗定位、过期、点击跳转 | notificationd 数据 + Shell role | 未开始 | 旧 UI 未迁入 |
| 输出管理 | 多屏、分辨率、缩放、旋转 | com.cutefish.Display | 未开始 | 接口边界已定义 |
| 任务切换 | Alt-Tab、窗口分组、任务栏同步 | core 窗口模型 + Shell | 未开始 | 未实现 |
| 安装器 | Calamares 全流程 | cutefish-installer 固定流程 | 进行中 | UI 骨架可用；磁盘 Job 禁止在测试服务器执行 |
| 应用 | filemanager/settings/terminal 等 | Qt Wayland 原生客户端 | 未开始 | app/ 默认冻结 |
