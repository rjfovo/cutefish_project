# 下次继续前先读我：当前交接快照

> 生成时间：2026-08-16
> 当前 git HEAD：f0a75d63
> 使用方式：下次会话先读本文件，再读 、、，
> 然后从“下次开始执行”继续。

## 0. 一句话状态

stage-0 已验收通过；stage-1 进行中。核心合成器已具备双 socket、Seat 输入、
窗口模型、xdg-shell 基础、剪贴板/主选择端到端、text-input-v3、QML 输出/窗口模型。
尚未完成发布级 compositor/shell 闭环，不能发布产品镜像。

## 1. 已经完成

### 1.1 工程与构建

- Usage: wayland-scripts/build_code.sh {project|all|clean [project]|list}

Projects from /workspace/cutefish_project/wayland-scripts/projects.list:
cutefish-compositor-core
cutefish-shell
cutefish-installer
core：
  - 旧脚本四种用法保持一致；
  - CUTEFISH_ROOT 可覆盖根目录；
  - 产物分类到 ；
  - dpkg 在  副本构建；
  - SOURCE_DATE_EPOCH、源 commit、日志、clean 边界均已验证。
-  当前项目：
  - cutefish-compositor-core
  - cutefish-shell
  - cutefish-installer
  - core

### 1.2 stage-0

- ：
  - session、settings-daemon、powerman、notificationd、polkit-agent、
    clipboard、cpufreq、screen-brightness、shutdown-ui；
  - 无 kwin-wayland/xwayland/SDDM 依赖，无 xsession 文件。
-  固定流程 QML + 危险 Job 安全拒绝。
-  target/live 包清单和移除清单。
- stage-0 验收通过。

### 1.3 stage-1 核心合成器

- VirtualBackend、KmsBackend 安全探针路径、GBM buffer。
- LibinputBackend、LogindSession 接口/实现。
- 双 Wayland socket：
  - apps socket 无 ；
  - shell socket 有 。
-  v5：
  - xkb keymap；
  - pointer/keyboard enter/leave；
  - key/motion/button；
  - axis、touch、repeat_info 基础事件。
-  / ：
  - 窗口 id、标题、appId、状态、激活、geometry；
  - z-order、move/resize 交互模型。
- xdg-shell：
  - toplevel 状态和 configure；
  - positioner/popup 初始 configure；
  - move/resize 请求接入。
- xdg_activation_v1：
  - token、activate、activated configure。
- 剪贴板/主选择：
  - ；
  - ；
  - 端到端 receive 测试通过。
- data-control：
  - vendor XML；
  -  source/device/offer 资源；
  - manager/source/device 创建测试通过。
- text-input：
  - ；
  - enable/disable → enter/leave + done；
  - 测试通过。

### 1.4 stage-1 Shell

-  连接 shell socket，绑定  v2；
- 输出/窗口事件接收；
-  用户会话 D-Bus 服务；
- 、；
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

- 每个迁移模块都有 ；
- 关键源码已补中文文件头/职责/边界注释；
- OK: stage-0/compositor-core.md
OK: stage-0/shell.md
OK: stage-0/installer.md
OK: stage-0/core-package.md
OK: stage-0/core-session.md
OK: stage-0/core-settings-daemon.md
OK: stage-0/core-powerman.md
OK: stage-0/core-notificationd.md
OK: stage-0/core-polkit-agent.md
OK: stage-0/core-clipboard.md
OK: stage-0/core-cpufreq.md
OK: stage-0/core-screen-brightness.md
OK: stage-0/core-shutdown-ui.md
OK: stage-1/backends.md
OK: stage-1/wm-protocol.md
OK: stage-1/seat-input.md
OK: stage-1/xdg-activation.md
OK: stage-1/shell-core-client.md
OK: stage-1/data-device.md
OK: stage-1/shell-qml-models.md
OK: stage-1/text-input.md
PASS: migration docs and comment hygiene checks 检查文档和旧注释；
- ---
Forbidden source hits in cutefish/wayland-code: 0
PASS: no forbidden X11/KWin/QtWaylandCompositor/wlroots symbols in new code. 强化了通用 X11/xcb 扫描；
-  即本文件。

## 2. 尚未完成

### 2.1 P0 协议

- ：
  - 未实现；
  - Debian/上游当前没有同名稳定 XML；
  - 等待 CONFLICT-007 决策。
- ：
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
- live ISO 直接启动  未实现。

### 2.5 硬件验证

- 物理 AMD/Intel GPU modeset/page flip 未验证；
- 真实键盘/鼠标/触控板到应用窗口端到端未验证；
- 睡眠恢复/VT 切换未验证。

### 2.6 阶段推进

- stage-1 未验收，不能进入 stage-2；
- stage-2/3/4 均未开始。

## 3. 当前阻塞

- ：
  - 测试服务器为 VMware 虚拟 GPU；
  - 物理 AMD/Intel GPU 复验等待设备或明确放宽。
- ：
  -  没有现成稳定 XML；
  - 需要决策采用实验协议/改写，或确认 v1+text-input-v3 组合。

## 4. 下次开始执行

建议顺序：

1. 打开并运行：
   - 
   - # CutefishOS Wayland 迁移实时进度

> 本文档目录是迁移执行期间唯一实时进度记录区。方案文档仍以 `doc/future/` 为准且只读。
> 状态取值只能为：`未开始`、`进行中`、`阻塞`、`验收通过`。
> 所有阻塞项必须指向 `conflicts.md` 中的冲突编号。

## 总进度

- 当前阶段：stage-1（进行中）
- 当前任务：P0 协议推进（剪贴板/文本输入）、move/resize 模型、Shell QML 模型、真机复验
- 阻塞项：CONFLICT-006 部分解决；VMware 虚拟 GPU 探针通过，AMD/Intel 物理 GPU
  模式设置/上屏复验仍待设备或明确放宽策略

## 当前状态摘要

| 工作流 | 状态 | 说明 |
|---|---|---|
| stage-0 Wayland-only baseline | 验收通过 | 见 `tasks/stage-0/*.md` |
| stage-1 compositor/shell 闭环 | 进行中 | 协议/WM/QML 模型推进；CONFLICT-006/007 待解决 |
| stage-2 login/lock | 未开始 | 待 stage-1 验收通过后进入 |
| stage-3 boot/shutdown | 未开始 | 待 stage-2 验收通过后进入 |
| stage-4 purge | 未开始 | 待 stage-3 验收通过后进入 |
| installer 专用化 | 进行中 | 固定流程 UI 骨架可用；磁盘 Job 禁止在当前环境执行 |

## 已完成并验证

1. `wayland-scripts/build_code.sh` 四种旧用法、根目录覆盖、分类输出、源码副本 dpkg 构建、
   SOURCE_DATE_EPOCH、日志和 clean 边界均验证通过。
2. `cutefish-compositor-core` VirtualBackend + 双 Wayland socket + cutefish_core_v1 +
   minimal xdg-shell；协议冒烟测试通过；Qt `cutefish-shell` 以 Wayland QPA 连接通过；
   `--kms` 安全拒绝通过。
3. `cutefish-shell` 五态 QML 骨架；offscreen QML 测试和 Wayland 连接测试通过。
4. `cutefish-installer` 固定流程 UI 骨架；危险 Job 安全拒绝；offscreen QML 测试通过。
5. `cutefish/wayland-code/core` Wayland-only 包边界迁移副本：
   session、settings-daemon、powerman、notificationd、polkit-agent、clipboard、
   cpufreq、screen-brightness、shutdown-ui；deb 无 kwin-wayland/xwayland/SDDM 依赖，
   无 X11 session 文件。
6. X11/KWin 源码符号扫描 0 命中；新包二进制直接依赖/符号扫描通过。
7. `wayland-scripts/iso/` 新 target/live 包清单和移除清单通过自检。

## 目录边界（执行层）

- 新源码：`cutefish/wayland-code/`
- 新构建脚本：`wayland-scripts/`
- 构建产物：`cutefish/output/build|packages|symbols|logs|cache/`
- 进度记录：`doc/migration/`
- 冻结只读：`cutefish/code/`、`script/`、`build_iso/`、`app/`、`doc/future/`

## 下一步

1. 完成 KmsBackend/LibinputBackend/LogindSession 编码与本地编译。
2. 在测试服务器 `/tmp/<test-dir>` 下运行非破坏性 KMS/GBM/libinput 能力探针并清理。
3. 继续实现最小协议集、WM 模型、最终合成与 Shell 窗口化。
4. 物理 AMD/Intel GPU 模式设置/上屏复验仍需专用设备或明确放宽策略（CONFLICT-006）。
5. stage-1 完成前不得发布产品镜像；无 X11/KWin/SDDM/Calamares 运行链回退。

## 未完成事项

- KMS/Libinput/Logind 真实硬件路径未实现，也未在任何环境执行 KMS 测试（见测试服务器边界）。
- 安装器磁盘 Job 未实现；只允许在专用硬件验证环境执行，禁止在测试服务器执行。
- 现有模块迁移副本：desktop/dock/statusbar/launcher/filemanager/fishui/qt-plugins/
  settings/screenshot/screenlocker/libcutefish/screen 按 stage-1 计划推进。
- `wayland-scripts/iso/` 目前为包清单和检查器，完整 live/target 构建脚本待实现。
   - # Stage 0~4 状态表

状态只能使用：未开始、进行中、阻塞、验收通过。阻塞必须指向 conflicts.md 编号。

| Stage | 状态 | 开始时间 | 验收状态 | 阻塞项 | 备注 |
|---|---|---|---|---|---|
| stage-0-wayland-only-baseline | 验收通过 | 2026-08-16 | 通过 | 无 | 工程基线、构建系统、core/shell/installer 骨架、core 包边界均验证通过 |
| stage-1-qt-compositor-shell | 进行中 | 2026-08-16 | 未验收 | CONFLICT-006（物理 GPU 复验）、CONFLICT-007（input-method-v2 XML） | Seat/输入链路、窗口模型、text-input-v3、QML 模型推进中 |
| stage-2-login-lock | 未开始 | - | 未验收 | 无 | 未进入 |
| stage-3-boot-shutdown | 未开始 | - | 未验收 | 无 | 未进入 |
| stage-4-purge | 未开始 | - | 未验收 | 无 | 未进入 |
| installer 专用化（并行） | 进行中 | 2026-08-16 | 未验收 | 无 | UI/工程骨架通过；磁盘 Job 未实现且禁止在测试服务器执行 |
   - # 冲突与阻塞记录

规则：冲突编号唯一；每条记录涉及文档/代码、停止点、等待决策。提示词优先级高于 `doc/future/`；
架构决策、功能范围、验收标准仍以方案文档为准。方案文档本身不修改。

## CONFLICT-001 目录路径映射冲突

- 涉及：`doc/future/*` 旧路径写法 vs 本任务目录边界
- 冲突：方案文档使用 `cutefish/code/<new-project>`、直接修改 `script/`、`build_iso/`；
  本任务规定新代码只能写入 `cutefish/wayland-code/`，新脚本只能写入 `wayland-scripts/`，
  ISO 等价脚本写入 `wayland-scripts/iso/`。
- 停止点：任何按方案旧路径写入的尝试。
- 决策：按任务目录映射补丁执行，不修改 `doc/future/`。
- 状态：已解决（执行层采用新路径，文档不改）。

## CONFLICT-002 stage-0 SDDM 过渡方案与“无过渡期”冲突

- 涉及：`stage-0-wayland-only-baseline/README.md` §2.2 建议“开发分支暂时保留 SDDM 方案1”；
  本任务最高原则/衍生决策要求 SDDM 不在产品运行链过渡保留，且 stage-0 验收要求产品构建不含 SDDM/X11 session。
- 停止点：新构建清单是否可包含 SDDM。
- 决策：以本任务为准。新 `wayland-scripts/iso/` 包清单不包含 SDDM/X11 session；
  开发树不发布包含 SDDM 的过渡镜像；LoginShell 是产品发布门槛。
- 状态：已解决（执行层不包含 SDDM）。

## CONFLICT-003 旧构建脚本与 ISO 脚本修改冲突

- 涉及：方案要求修改 `script/build_code.sh`、`script/create_iso/*`、`build_iso/*`；
  本任务冻结旧 `script/`、`build_iso/`。
- 停止点：是否原地修改旧脚本。
- 决策：在 `wayland-scripts/` 下新建等价脚本；ISO 等价配置放 `wayland-scripts/iso/`。
- 状态：已解决。

## CONFLICT-004 “删除 X11/KWin 实现”与旧工作树只读冲突

- 涉及：各阶段“删除 X11/KWin 代码/组件”与“cutefish/code/ 只读、禁止删除”冲突。
- 停止点：物理删除旧代码。
- 决策：按任务定义执行：新代码/迁移副本中不存在对应实现；旧实现不进入新构建清单和 ISO 包清单；
  `cutefish/code/` 保持只读。物理删除不做。
- 状态：已解决（作为长期执行定义）。

## CONFLICT-005 stage-0 要求直接修改 cutefish/code/core 与冻结边界冲突

- 涉及：`stage-0-wayland-only-baseline/README.md` 要求直接修改 `cutefish/code/core` 拆包；
  本任务禁止修改 `cutefish/code/`。
- 停止点：是否原地修改旧 core。
- 决策：在 `cutefish/wayland-code/core` 建立迁移副本并重写 CMake/debian；
  旧 `cutefish/code/core` 只读，仅作审计参考。
- 状态：已解决。

## CONFLICT-006 KMS Spike 真机验证硬件边界

- 涉及：`cutefish-dedicated-compositor-architecture.md` §18 要求 KMS 相关 Spike 在
  AMD/Intel 真机复验；原任务测试服务器边界禁止 DRM/input 验证。
- 停止点：stage-1 KmsBackend 验证。
- 决策：用户已授权“去测试环境做真机验证”。2026-08-16 对 192.168.118.132 执行了
  只读审计和安全探针：
  - 服务器为 VMware 虚拟 GPU（vendor 0x15ad/device 0x0405），宿主机 CPU 为
    AMD Ryzen 7 2700X；不是物理 AMD/Intel GPU。
  - `drmModeGetResources` 探针通过：8 CRTC/8 connector，1 个 connected connector。
  - GBM 在 renderD128 上创建 1920x1080 XRGB8888 scanout/rendering BO 通过。
  - 未执行 drmSetMaster、modeset、page flip、input grab，未影响当前 SDDM/Xorg/KWin 会话。
- 状态：部分解决。测试服务器可继续执行非破坏性 KMS/GBM/libinput 能力探针；
  方案要求的 AMD/Intel 物理 GPU 模式设置/上屏复验仍待设备或明确放宽策略。
- 当前执行策略：stage-1 继续开发；KmsBackend 默认只枚举和分配 GBM，不申请 DRM master；
  模式设置/上屏路径必须显式设置 `CUTEFISH_KMS_ALLOW_MODESET=1` 且只能在批准的
  专用测试机/维护窗口执行。

## CONFLICT-007 zwp_input_method_v2 XML 与键盘 grab 设计缺失

- 涉及：`development-key-points.md` P0 要求实现 `zwp_input_method_v2`；
  当前 Debian 13 `wayland-protocols` 仅提供 `input-method-unstable-v1.xml`。
- 停止点：不能以 v1 冒充 v2，也不能在未定义键盘 grab 状态机前实现 P0。
- 等待决策：上游当前没有以 `zwp_input_method_v2` 命名的稳定 v2 XML；
  Debian 只有 `input-method-unstable-v1.xml`，最新实验协议为
  `xx_input_method_v2`。需批准采用/改写协议或确认兼容 v1 + text-input-v3 组合。
  同时需确认 fcitx5/ibus 键盘 grab、候选窗 surface、锁屏停止输入状态机。
- 已执行：vendor 重构版 `wlr-data-control-unstable-v1.xml` 至
  `cutefish/wayland-code/cutefish-compositor-core/protocol/vendor/`；
  上游 wlroots XML 当前网络不可达，文件为 v1 最小接口重构。
- 状态：未解决。
2. 验证当前树可构建：
   - ==> Building project 'cutefish-compositor-core' (type=dpkg)
========================================================================
PROJECT: cutefish-compositor-core
SOURCE: /workspace/cutefish_project/cutefish/wayland-code/cutefish-compositor-core
OUTPUT: /workspace/cutefish_project/cutefish/output/build/cutefish-compositor-core
DPKG SOURCE COPY: /workspace/cutefish_project/cutefish/output/build/cutefish-compositor-core/source
LOG FILE: /workspace/cutefish_project/cutefish/output/logs/cutefish-compositor-core/build.log
BUILD COMMAND: dpkg-buildpackage -b -uc -us (run in source copy)
SOURCE COMMIT: f0a75d631bac007df911b6a52ed094c142d5c9e4
SOURCE_DATE_EPOCH: 1786890077
========================================================================
source=/workspace/cutefish_project/cutefish/wayland-code/cutefish-compositor-core
source_commit=f0a75d631bac007df911b6a52ed094c142d5c9e4
BUILD EXIT CODE: 0
==> Building project 'cutefish-shell' (type=dpkg)
========================================================================
PROJECT: cutefish-shell
SOURCE: /workspace/cutefish_project/cutefish/wayland-code/cutefish-shell
OUTPUT: /workspace/cutefish_project/cutefish/output/build/cutefish-shell
DPKG SOURCE COPY: /workspace/cutefish_project/cutefish/output/build/cutefish-shell/source
LOG FILE: /workspace/cutefish_project/cutefish/output/logs/cutefish-shell/build.log
BUILD COMMAND: dpkg-buildpackage -b -uc -us (run in source copy)
SOURCE COMMIT: f0a75d631bac007df911b6a52ed094c142d5c9e4
SOURCE_DATE_EPOCH: 1786890077
========================================================================
source=/workspace/cutefish_project/cutefish/wayland-code/cutefish-shell
source_commit=f0a75d631bac007df911b6a52ed094c142d5c9e4
BUILD EXIT CODE: 0
==> Building project 'cutefish-installer' (type=dpkg)
========================================================================
PROJECT: cutefish-installer
SOURCE: /workspace/cutefish_project/cutefish/wayland-code/cutefish-installer
OUTPUT: /workspace/cutefish_project/cutefish/output/build/cutefish-installer
DPKG SOURCE COPY: /workspace/cutefish_project/cutefish/output/build/cutefish-installer/source
LOG FILE: /workspace/cutefish_project/cutefish/output/logs/cutefish-installer/build.log
BUILD COMMAND: dpkg-buildpackage -b -uc -us (run in source copy)
SOURCE COMMIT: f0a75d631bac007df911b6a52ed094c142d5c9e4
SOURCE_DATE_EPOCH: 1786890077
========================================================================
source=/workspace/cutefish_project/cutefish/wayland-code/cutefish-installer
source_commit=f0a75d631bac007df911b6a52ed094c142d5c9e4
BUILD EXIT CODE: 0
==> Building project 'core' (type=dpkg)
========================================================================
PROJECT: core
SOURCE: /workspace/cutefish_project/cutefish/wayland-code/core
OUTPUT: /workspace/cutefish_project/cutefish/output/build/core
DPKG SOURCE COPY: /workspace/cutefish_project/cutefish/output/build/core/source
LOG FILE: /workspace/cutefish_project/cutefish/output/logs/core/build.log
BUILD COMMAND: dpkg-buildpackage -b -uc -us (run in source copy)
SOURCE COMMIT: f0a75d631bac007df911b6a52ed094c142d5c9e4
SOURCE_DATE_EPOCH: 1786890077
========================================================================
source=/workspace/cutefish_project/cutefish/wayland-code/core
source_commit=f0a75d631bac007df911b6a52ed094c142d5c9e4
BUILD EXIT CODE: 0
   - Internal ctest changing into directory: /workspace/cutefish_project/cutefish/output/build/cutefish-compositor-core/cmake
Test project /workspace/cutefish_project/cutefish/output/build/cutefish-compositor-core/cmake
    Start 1: wayland_protocol_smoke
1/1 Test #1: wayland_protocol_smoke ...........   Passed    1.23 sec

100% tests passed, 0 tests failed out of 1

Total Test time (real) =   1.23 sec
   - Internal ctest changing into directory: /workspace/cutefish_project/cutefish/output/build/cutefish-shell/cmake
Test project /workspace/cutefish_project/cutefish/output/build/cutefish-shell/cmake
    Start 1: shell_qml_offscreen_smoke
1/1 Test #1: shell_qml_offscreen_smoke ........   Passed    1.17 sec

100% tests passed, 0 tests failed out of 1

Total Test time (real) =   1.18 sec
3. 先处理 data-control selection 端到端测试，关闭剪贴板链路。
4. 再处理 CONFLICT-007 输入法协议决策。
5. 之后继续：
   - xdg_popup 完整行为；
   - 窗口 move/resize 与真实指针输入联调；
   - Renderer/最终合成；
   - Shell PAM/登录锁屏；
   - 安装器 polkit helper 与 Job 清单。
6. 每次逻辑步骤后：
   - 更新 ；
   - 运行构建/测试；
   - 单独 git commit。

## 5. 禁止事项提醒

- 不修改：、、、、；
- 不引入：X11 session、XWayland、KWin、KDE Plasma、QtWaylandCompositor API、
  eglfs_kms、wlroots、SDDM、Calamares 产品运行链；
- 不在测试服务器执行：系统修改、DRM master/modeset、input grab、
  分区/格式化/grub/efivarfs、影响生产会话的操作；
- 不把测试服务器密码写入 git、文档或日志。
