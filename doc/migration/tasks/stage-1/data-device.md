# stage-1 任务：wl_data_device / primary_selection 剪贴板链路

- 状态：进行中

## 模块迁移元数据

- 原项目路径：无（新增协议实现）
- 迁移后路径：`cutefish/wayland-code/cutefish-compositor-core/wayland/data_device.*`
- 迁移前依赖：无
- 迁移后依赖：libwayland-server、wayland-protocols primary-selection
- 架构变化：selection 缓存进入 core；主选择与剪贴板独立 store
- 对外接口变化：`wl_data_device_manager`、`zwp_primary_selection_device_manager_v1`
- 功能差异/裁剪：DnD 尚未实现；`zwlr_data_control_v1` 待 vendored XML
- 测试与验收：source/device 创建、selection offer、异步缓存、receive 回读端到端 PASS

## 实际修改

- `DataDeviceManager` 注册两个全局。
- `wl_data_source/device/offer` 与 primary source/device/offer 资源实现。
- selection 源异步读取：pipe + run-loop poll，MIME 顺序缓存，上限 16MiB。
- `receive` 从 core 缓存写回客户端 fd。
- WaylandServer run loop 纳入 pending read fd。

## 测试与验收

- `wayland_protocol_smoke` 验证 manager/source/device 创建：PASS。
- 修复协议顺序：先 `data_offer`(new_id) → `offer`(mime) → `selection`(object)。
- 剪贴板写入 → core 缓存 → `receive` 回读 `hello-cutefish-clipboard`：PASS。
- 主选择写入 → core 主选择缓存 → primary offer receive 回读：PASS。
