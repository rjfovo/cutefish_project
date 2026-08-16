# 模块迁移文档：polkit-agent

## 1. 路径
- 原项目路径：`cutefish/code/core/polkit-agent`
- 迁移后路径：`cutefish/wayland-code/core/polkit-agent`

## 2. 依赖变化
- 迁移前后：Qt Core/Widgets/Quick/QuickControls2、polkit-qt6-1；无 X11 依赖变化。

## 3. 架构变化
- 保持用户会话 polkit 认证代理职责。
- 仅清理旧构建路径；运行位置仍为图形会话。

## 4. 对外接口变化
- 无。

## 5. 功能差异与首版裁剪项
- 无首版裁剪。

## 6. 测试与验收状态
- 编译通过并打入 cutefish-core；未做系统级 polkit 授权测试。
