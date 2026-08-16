# 模块迁移文档：screen-brightness

## 1. 路径
- 原项目路径：`cutefish/code/core/screen-brightness`
- 迁移后路径：`cutefish/wayland-code/core/screen-brightness`

## 2. 依赖变化
- 迁移前后：Qt Core/Gui/Widgets；无 X11 依赖。

## 3. 架构变化
- 保持 pkexec helper 形态；后续亮度硬件路径收敛到 core DPMS/背光接口。

## 4. 对外接口变化
- polkit action 保持。

## 5. 功能差异与首版裁剪项
- 无。

## 6. 测试与验收状态
- 编译通过；未执行真实亮度变更。
