# stage-1 任务：text-input-v3 与 zwp_input_method_v2（P0）

- 状态：进行中

## 模块迁移元数据

- 原项目路径：无（新增协议实现）
- 迁移后路径：`cutefish/wayland-code/cutefish-compositor-core/wayland/text_input.*`
- 迁移前依赖：不适用
- 迁移后依赖：wayland-protocols text-input-unstable-v3
- 架构变化：core 持有 text-input 焦点；启用时发送 enter+done
- 对外接口变化：`zwp_text_input_manager_v3`
- 功能差异/裁剪：候选窗、delete_surrounding_text、提交状态未实现
- 测试与验收：enable→done 协议测试通过；input-method-v2 被 CONFLICT-007 阻塞

## 实际修改

- 注册 `zwp_text_input_manager_v3` 和 `zwp_text_input_v3` 资源。
- enable/disable 关联 Workspace active surface，发送 enter/leave 和 done。
- set_* 请求暂存边界保留；commit 回复 done。

## 测试结果

- `wayland_protocol_smoke`：manager/text_input 创建、enable→done PASS。

## 阻塞

- `zwp_input_method_v2`：Debian wayland-protocols 无 v2 XML，见 CONFLICT-007。
