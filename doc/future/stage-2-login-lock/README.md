# 阶段 2：Login/LockShell 与去 SDDM

> 所属主计划：`../wayland-migration-master-plan.md`
> 架构依据：`../cutefish-dedicated-compositor-architecture.md`
> 目标：`cutefish-shell` 实现登录和锁屏 UI；`cutefish-compositor-core` 持有会话和锁定状态；去掉 SDDM。

## 1. 需要修改的项目

| 项目/路径 | 修改内容 |
|---|---|
| `cutefish-compositor-core/auth` | PAM helper、logind 会话注册、自动登录 |
| `cutefish-compositor-core/lifecycle` | Login/Session/Lock 状态机；ShellManager |
| `cutefish-compositor-core/portal` | 锁屏期间截图/输入授权 |
| `cutefish-shell/LoginShell` | 用户列表、密码、会话选择、错误处理 |
| `cutefish-shell/LockShell` | 锁屏 UI；core 持有锁定状态和安全 fallback |
| `cutefish-shell/SessionShell` | 用户会话 Shell |
| `core/session` | 删除 SDDM 假设；由 core 启动用户服务 |
| `sddm-theme` | 删除模块 |
| `build_iso` | 移除 SDDM 包和配置 |
| `screenlocker` | 删除旧锁屏进程 |

## 2. 依赖

```text
PAM
systemd/logind
accountsservice（用户列表可选）
Qt6::DBus
Qt6::Quick
qt6-wayland（Shell 客户端）
libwayland-server（core）
```

不再依赖：

```text
sddm
sddm-theme
KScreenLocker
Qt6::WaylandCompositor
```

## 3. 需要解决的问题

1. SDDM 当前负责用户选择、密码验证、会话启动和自动登录。
2. 登录/注销不得重启 `cutefish-compositor-core`，只切换 Shell。
3. PAM 必须走独立 helper，不能在渲染线程调用。
4. 双 socket 模型：登录后 Shell 走专用 socket，普通应用走标准 socket。
5. 进入 Lock 后：
   - core 停止合成所有普通应用 surface；
   - LockShell 未就绪或 Shell 崩溃时，core 显示安全 fallback 帧；
   - 普通客户端不能接收输入；
   - 窗口缩略图/纹理导出禁用。
5. 首版用户切换定义为“注销并回到 LoginShell”，不实现并发多用户会话。
6. 登录失败、账户锁定、认证超时需要一致错误界面和审计日志。
7. 注销后 core 清理用户服务，回到 LoginShell：
   terminate → 超时 SIGKILL → 强制断开 → 清理残留 surface。

## 4. 功能变化

| 功能 | 变化 |
|---|---|
| 登录 | SDDM → `cutefish-shell --greeter` |
| 自动登录 | sddm 配置 → core auth 配置 |
| 会话注册 | sddm-helper → logind |
| 锁屏 | 旧独立客户端 → core 锁定状态 + Shell LockShell |
| 注销 | 退出 session Shell，core 回到 Login 状态 |
| 用户切换 | 首版为注销并回到登录界面 |

## 5. 可能遇到的问题

| 问题 | 应对 |
|---|---|
| PAM 安全 | PAM helper 独立进程；core 只通过受控 IPC 获取结果 |
| logind 会话 | 登录成功后注册用户会话；注销逆序清理 |
| LockShell 绕过 | core 直接停止普通应用合成；fallback 帧由 core 维护 |
| 登录前无用户 D-Bus | core 运行系统级；登录后启动 user@.service |
| 自动登录配置迁移 | 不读 sddm.conf；使用 Cutefish 配置 |
| Shell 崩溃 | core 保持 Login/Lock 状态并重启 Shell |

## 6. 验收标准

- 登录、注销、自动登录、锁屏/解锁全链路通过；
- 登录/注销过程中 core 不重启、无黑屏/闪烁；
- 锁屏后普通应用不可见、不可输入；
- LockShell 未就绪时显示 core fallback 帧；
- 系统无 SDDM、Xorg、XWayland、KWin。

## 7. 交付物

1. LoginShell、LockShell、SessionShell。
2. core auth/lifecycle/ShellManager。
3. 去 SDDM 的 ISO。
4. 登录/锁屏安全和故障测试报告。
