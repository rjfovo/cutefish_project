# stage-1 任务：KmsBackend / LibinputBackend / LogindSession

- 状态：进行中
- 方案依据：`cutefish-dedicated-compositor-architecture.md` §6/§13

## 实际修改

- `backend/kms_backend.h/cpp`：
  - card0 打开、DRM resource/connector/mode 枚举；
  - GBM device/buffer 创建；
  - master/modeset/page-flip 路径由 `CUTEFISH_KMS_ALLOW_MODESET=1` + `--kms-modeset` 门控；
  - DPMS connector property 写入、cursor stub。
- `input/libinput_backend.h/cpp`：
  - udev seat 初始化、path/libinput fd 与 dispatch 边界、键盘布局/输入注入接口。
- `session/logind_session.h/cpp`：
  - logind manager/seat/session D-Bus 封装（TakeControl 检查、Suspend、SwitchTo、Lock、PowerOff）。
- `main.cpp`：`--kms` 进入 KmsBackend 安全探针模式；未授权 modeset 时只枚举和创建 Wayland socket。

## 测试结果

### 测试服务器 192.168.118.132（只读/非破坏性探针）

- 硬件：VMware 虚拟 GPU（vendor 0x15ad/device 0x0405）；宿主机 CPU AMD Ryzen 7 2700X。
- DRM probe：dumb-buffer 支持；8 CRTC/8 connector；1 个 connected connector（CONNECTOR-41，1280x800，32 modes）。
- 本机编译并上传 `/tmp/cutefish-compositor-core-stage1`：
  `sudo env XDG_RUNTIME_DIR=/tmp/cutefish-kms-run timeout 4 ... --kms --runtime-dir ...`
  退出码 124（timeout 终止），日志显示 KmsBackend 枚举成功、Wayland server 启动；
  未调用 drmSetMaster/modeset/page flip；测试后 `/tmp` 清理，SDDM/Xorg/KWin 会话保持正常。
- GBM probe：renderD128 创建 1920x1080 XRGB8888 scanout+rendering BO 成功。
- libinput probe：本地编译后上传 `/tmp/cutefish-inputprobe`，sudo 执行；seat0 分配成功，
  枚举 5 个设备（电源按钮、VMware 鼠标、AT 键盘、两个 VMMouse）；无 input grab；
  测试后清理，会话保持正常。
- 本地 VirtualBackend CTest：PASS。

## 验收状态

- 未验收。待完成：真实 modeset/page flip、libinput 事件到 Wayland seat、logind 生命周期联调。
- CONFLICT-006：AMD/Intel 物理 GPU 复验仍待设备或明确放宽策略。

## 模块迁移元数据

- 原项目路径：无（新增 backend 实现）
- 迁移后路径：`cutefish/wayland-code/cutefish-compositor-core/{backend,input,session}`
- 迁移前依赖：不适用
- 迁移后依赖：libdrm、gbm、libinput、libudev、QtDBus
- 架构变化：DisplayBackend/InputBackend/SessionBackend 接口化
- 对外接口变化：`--kms` 安全探针、`CUTEFISH_KMS_ALLOW_MODESET` 门控
- 功能差异/裁剪：未执行真实 modeset/page flip；VMware 能力探针通过
- 测试与验收：本地编译/CTest、测试服务器 KMS/GBM/libinput 探针通过；真机 modeset 未验收
