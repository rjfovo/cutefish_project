# CutefishOS 开机/关机 Plymouth 动画设计

> 最后更新：2026-08-15
> 目标：替换原先“一张静态 logo”的 demo 感，设计符合 Cutefish 设计语言的
> 开机、关机、重启过场动画。

## 1. 设计关键词

- 深邃、安静、可信赖
- 与 SDDM 登录界面同源：暗色、半透明、品牌蓝
- 开机 = 唤醒；关机 = 退场；两者逻辑相反但视觉语言一致

## 2. 视觉构成

| 层级 | 内容 | 说明 |
|---|---|---|
| 背景 | Cutefish 默认壁纸的高斯模糊+提亮+82% 白色混合，按屏幕比例裁剪 | 白/浅灰，与 Cutefish 桌面和 SDDM 登录风格一致 |
| 底色 | `#F5F7FA → #DEE3EA` 浅灰蓝渐变兜底 | 背景图加载失败时仍保持品牌感 |
| 环境光 | 两层极低透明度白色柔光 | 只增加轻微空气感，不再出现霓虹式亮斑 |
| 主视觉 | Cutefish 原版 wordmark | 水平居中，约 40% 屏高；1.05Hz 轻微上下浮动 |
| 光晕 | logo 后方的蓝色 halo | 随 logo 呼吸，透明度 0.30~0.50 |
| 进度 | 220~430px 细进度条 | 位于约 66% 屏高；有移动高光、末端亮点 |
| 密码框 | 半透明深色圆角卡片 | 与 SDDM 登录卡片一致；锁、输入框、圆点 |

## 3. 动画逻辑

### 3.1 开机 / 重启（boot / reboot）

```text
0.0s   背景渐亮，两团青蓝色 bloom 开始漂移
0.2s   logo 淡入到 88% 不透明度
之后    logo 以 1.05Hz 上下浮动 ±0.4% 屏高
       halo 以 1.35Hz 呼吸
       bloom 以 0.12Hz / 0.10Hz 缓慢漂移
       进度条未知时以 1.7Hz 做呼吸式长度变化
       高光每 0.1s 前移 3px，形成流动感
```

### 3.2 关机（shutdown）

- 主构图不变，但环境光、进度条、tip 全部切换为**暖琥珀色**
  （`#FFB85C → #FF6046`）。
- 用户一眼即可区分“正在进入系统”和“正在离开系统”。
- 动画节奏降低为更慢的呼吸，视觉上更像逐渐熄灭。

### 3.3 重启（reboot）

- 使用开机青蓝色，表示系统即将重新唤醒。

### 3.4 LUKS 密码输入

- logo 上移到 14% 屏高并保持清晰。
- 背景 bloom 压暗到 0.10/0.07，减少干扰。
- 中央显示与 SDDM 同语言的半透明圆角卡片。
- 输入完成后恢复主构图。

## 4. 技术实现

- 继续使用 Plymouth `script` 插件：可全屏背景、可随窗口尺寸缩放、
  可用 `Plymouth.SetRefreshRate(30)` 驱动帧动画。
- `Plymouth.GetMode()` 区分 `boot` / `shutdown` / `reboot`，动态选择调色板。
- 开机使用 `SetBootProgressFunction` + `SetSystemUpdateFunction`，initramfs
  和 systemd 都能驱动进度条。
- 进度条通过 `Plymouth.SetBootProgressFunction` 接收真实进度；
  未知进度时自动退化到呼吸动画。
- 新增 12 张小尺寸 PNG 精灵，总包体约 120KB，initramfs 开销很小。

## 5. 文件

```text
cutefish/code/plymouth-theme/cutefish-logo/
├── cutefish-logo.plymouth
├── cutefish-logo.script
├── background.png
├── logo.png
├── logo-halo.png
├── glow-primary.png
├── glow-secondary.png
├── glow-shutdown.png
├── progress-track.png
├── progress-fill-boot.png
├── progress-fill-shutdown.png
├── progress-shimmer.png
├── progress-tip-boot.png
├── progress-tip-shutdown.png
└── box.png / entry.png / lock.png / bullet.png
```

## 6. 自动登录 Session Splash

自动登录时 SDDM 不会停留在登录界面，X ready 到桌面第一帧之间需要一个过渡层：

- `cutefish-session` 启动时立即显示全屏 `splash.qml`；
- 背景使用共享 `boot-background.png`，中间为 Cutefish logo；
- `cutefish-desktop` 启动完成后 3s 淡出；
- 12s 超时强制关闭，避免异常时卡在过渡图。
- 开机进度条封顶 86%，initramfs 进度封顶 78%，避免“100% 后还黑屏”的
  错误暗示。

## 7. 验证

- `script` 插件解析测试：boot/shutdown 两种模式均无 parse error。
- 已更新 initramfs；实际开机与关机由测试机验证。
