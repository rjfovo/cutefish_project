# stage-0 任务：wayland-scripts 构建体系

- 状态：进行中
- 方案依据：`doc/future/stage-0-wayland-only-baseline/README.md`
- 边界：不修改旧 `script/build_code.sh`；新脚本位于 `wayland-scripts/`。

## 实际修改

- `wayland-scripts/build_code.sh`、`lib-build.sh`、`projects.list`
- `wayland-scripts/check_x11_symbols.sh`、`check_binary_symbols.sh`
- `wayland-scripts/iso/{target-packages,list,live-packages.list,removed-from-product.list,check-manifests.sh,cutefish-installer.desktop}`

## 执行命令

```text
./wayland-scripts/build_code.sh list
./wayland-scripts/build_code.sh cutefish-compositor-core
./wayland-scripts/build_code.sh cutefish-shell
./wayland-scripts/build_code.sh cutefish-installer
./wayland-scripts/build_code.sh core
./wayland-scripts/build_code.sh clean cutefish-compositor-core
./wayland-scripts/build_code.sh clean
./wayland-scripts/build_code.sh all
```

## 测试结果

- `all` 构建 4 个项目全部退出码 0；deb/dbgsym/changes/buildinfo 分类正确。
- `clean <project>` 与 `clean` 只删除 `cutefish/output/{build,packages,symbols,logs}` 新目录；
  `build.cache` 文件保持，旧 `debs/dbgsym/build_log/code` 产物保持不变（36/27/70 前后一致）。
- `CUTEFISH_ROOT` 默认按 `wayland-scripts/..` 解析，无绝对路径硬编码。
- dpkg 构建在 `cutefish/output/build/<project>/source` 副本执行，副本记录源 commit。

## 验收状态

- 构建脚本行为：验收通过（本任务项）。
- 整体 stage-0：未验收（core 服务迁移副本未全部完成）。
