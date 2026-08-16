# 模块迁移文档：clipboard

## 1. 路径
- 原项目路径：`cutefish/code/core/clipboard`
- 迁移后路径：`cutefish/wayland-code/core/clipboard`

## 2. 依赖变化
- 迁移前后：Qt Core/Gui/Widgets；无 X11 显式依赖。

## 3. 架构变化
- stage-0 暂时保留旧客户端二进制以维持包边界；架构目标是把剪贴板管理移入 compositor core。
- 新代码不调用 X11 API。

## 4. 对外接口变化
- 无。

## 5. 功能差异与首版裁剪项
- 阶段迁移项：后续由 core `wl_data_device_manager` + `zwlr_data_control_v1` 替代。

## 6. 测试与验收状态
- 编译通过；未做剪贴板协议验收。
