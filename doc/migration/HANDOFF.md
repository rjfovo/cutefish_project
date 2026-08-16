# 下次继续前先读我：当前交接快照

> 生成时间：2026-08-16
> 当前 git HEAD：cc6d468a
> 使用方式：下次会话先读本文件，再读 `README.md`、`stage-status.md`、`conflicts.md`，
> 然后从“下次开始执行”继续。

## 0. 一句话状态

stage-0 已验收通过；stage-1 进行中。核心合成器已具备双 socket、Seat 输入、
窗口模型、xdg-shell 基础、剪贴板/主选择端到端、text-input-v3、QML 输出/窗口模型。
尚未完成发布级 compositor/shell 闭环，不能发布产品镜像。

## 1. 已经完成

### 1.1 工程与构建

- `wayland-scripts/build_code.sh`：
  - 旧脚本四种用法保持一致；
  - `CUTEFISH_ROOT` 可覆盖根目录；
  - 产物分类到 `cutefish/output/{build,packages,symbols,logs,cache}`；
  - dpkg 在 `output/build/<project>/source` 副本构建；
  - `SOURCE_DATE_EPOCH`、源 commit、日志、clean 边界均已验证。
- `wayland-scripts/projects.list` 当前项目：
  - `cutefish-compositor-core`
  - `cutefish-shell`
  - `cutefish-installer`
  - `core`

### 1.2 stage-0

- `cutefish/wayland-code/core`：
  - `session`、`settings-daemon`、`powerman`、`notificationd`、`polkit-agent`、
    `clipboard`、`cpufreq`、`screen-brightness`、`shutdown-ui`；
  - 无 `kwin-wayland`/`xwayland`/SDDM 依赖，无 xsession 文件。
- `cutefish-installer` 固定流程 QML + 危险 Job 安全拒绝。
- `wayland-scripts/iso/` target/live 包清单和移除清单。
- stage-0 验收通过。

### 1.3 stage-1 核心合成器

- `VirtualBackend`、`KmsBackend` 安全探针路径、GBM buffer。
- `LibinputBackend`、`LogindSession` 接口/实现。
- 双 Wayland socket：
  - apps socket 无 `cutefish_core_v1`；
  - shell socket 有 `cutefish_core_v1`。
- `wl_seat` v5：
  - xkb keymap；
  - pointer/keyboard enter/leave；
  - key/motion/button；
  - axis、touch、repeat_info 基础事件。
- `Window` / `Workspace`：
  - 窗口 id、标题、appId、状态、激活、geometry；
  - z-order、move/resize 交互模型。
- xdg-shell：
  - toplevel 状态和 configure；
  - positioner/popup 初始 configure；
  - move/resize 请求接入。
- `xdg_activation_v1`：
  - token、activate、activated configure。
- 剪贴板/主选择：
  - `wl_data_device_manager`；
  - `zwp_primary_selection_device_manager_v1`；
  - 端到端 receive 测试通过。
- data-control：
  - vendor XML；
  - `zwlr_data_control_manager_v1` source/device/offer 资源；
  - manager/source/device 创建测试通过。
- text-input：
  - `zwp_text_input_manager_v3`；
  - enable/disable → enter/leave + done；
  - 测试通过。

### 1.4 stage-1 Shell

- `ShellCoreClient` 连接 shell socket，绑定 `cutefish_core_v1` v2；
- 输出/窗口事件接收；
- `com.cutefish.Display` 用户会话 D-Bus 服务；
- `OutputListModel`、`WindowListModel`；
- QML Desktop/Dock/StatusBar/Launcher 已开始消费模型；
- offscreen QML 测试通过。

### 1.5 测试服务器验证

- KMS connector/mode 枚举：PASS；
- GBM buffer 创建：PASS；
- libinput seat/设备枚举：PASS；
- 未执行 drmSetMaster/modeset/page flip/input grab；
- 当前生产会话未受影响；
- 服务器是 VMware 虚拟 GPU，不是物理 AMD/Intel GPU。

### 1.6 文档与注释

- 每个迁移模块都有 `doc/migration/tasks/<stage>/<module>.md`；
- 关键源码已补中文文件头/职责/边界注释；
- `wayland-scripts/check_migration_docs.sh` 检查文档和旧注释；
- `wayland-scripts/check_x11_symbols.sh` 强化了通用 X11/xcb 扫描；
- `doc/migration/HANDOFF.md` 即本文件。

## 2. 尚未完成

### 2.1 P0 协议

- `zwp_input_method_v2`：
  - 未实现；
  - Debian/上游当前没有同名稳定 XML；
  - 等待 `CONFLICT-007` 决策。
- `zwlr_data_control_v1`：
  - 资源骨架完成；
  - selection 端到端 receive 测试尚未开启；
  - primary_selection/版本 2 扩展未做。

### 2.2 WM/合成

- 真实 modeset/page flip 未验证；
- move/resize 只完成模型，未接最终渲染和 CSD 交互验证；
- xdg_popup 完整约束求解、grab、dismiss 未完成；
- 最终合成器 Renderer 未实现；
- 窗口级模糊/圆角/阴影/动画未实现；
- 缩略图/Dock 预览/Alt-Tab 纹理导出未实现。

### 2.3 Shell

- Boot/Login/Lock/Session/Shutdown 仍主要是 QML 骨架；
- 真实登录 PAM、锁屏安全、通知 surface、AppMenu 未实现；
- Launcher 应用模型未接；
- Dock 关闭/最小化/预览交互未完成。

### 2.4 安装器

- UI 骨架完成；
- polkit helper、分区/格式化/rootfs 复制/grub/efivarfs Job 未实现；
- live ISO 直接启动 `cutefish-installer` 未实现。

### 2.5 硬件验证

- 物理 AMD/Intel GPU modeset/page flip 未验证；
- 真实键盘/鼠标/触控板到应用窗口端到端未验证；
- 睡眠恢复/VT 切换未验证。

### 2.6 阶段推进

- stage-1 未验收，不能进入 stage-2；
- stage-2/3/4 均未开始。

## 3. 当前阻塞

- `CONFLICT-006`：
  - 测试服务器为 VMware 虚拟 GPU；
  - 物理 AMD/Intel GPU 复验等待设备或明确放宽。
- `CONFLICT-007`：
  - `zwp_input_method_v2` 没有现成稳定 XML；
  - 需要决策采用实验协议/改写，或确认 v1+text-input-v3 组合。

## 4. 下次开始执行

建议顺序：

1. 打开并运行：
   - `cat doc/migration/HANDOFF.md`
   - `cat doc/migration/README.md`
   - `cat doc/migration/stage-status.md`
   - `cat doc/migration/conflicts.md`
2. 验证当前树可构建：
   - `./wayland-scripts/build_code.sh all`
   - `ctest --test-dir cutefish/output/build/cutefish-compositor-core/cmake --output-on-failure`
   - `ctest --test-dir cutefish/output/build/cutefish-shell/cmake --output-on-failure`
3. 先处理 data-control selection 端到端测试，关闭剪贴板链路。
4. 再处理 `CONFLICT-007` 输入法协议决策。
5. 之后继续：
   - xdg_popup 完整行为；
   - 窗口 move/resize 与真实指针输入联调；
   - Renderer/最终合成；
   - Shell PAM/登录锁屏；
   - 安装器 polkit helper 与 Job 清单。
6. 每次逻辑步骤后：
   - 更新 `doc/migration/tasks/...`；
   - 运行构建/测试；
   - 单独 git commit。

## 5. 禁止事项提醒

- 不修改：`cutefish/code/`、`script/`、`build_iso/`、`app/`、`doc/future/`；
- 不引入：X11 session、XWayland、KWin、KDE Plasma、QtWaylandCompositor API、
  eglfs_kms、wlroots、SDDM、Calamares 产品运行链；
- 不在测试服务器执行：系统修改、DRM master/modeset、input grab、
  分区/格式化/grub/efivarfs、影响生产会话的操作；
- 不把测试服务器密码写入 git、文档或日志。
