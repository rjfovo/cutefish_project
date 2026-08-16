# 冲突与阻塞记录

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
