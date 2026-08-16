# 模块迁移文档：cpufreq

## 1. 路径
- 原项目路径：`cutefish/code/core/cpufreq`
- 迁移后路径：`cutefish/wayland-code/core/cpufreq`

## 2. 依赖变化
- 迁移前后：Qt Core/Quick/DBus；无变化。

## 3. 架构变化
- 无。

## 4. 对外接口变化
- polkit action 和 pkexec 策略保持。

## 5. 功能差异与首版裁剪项
- 无。

## 6. 测试与验收状态
- 编译通过；未执行实际调频验证。
