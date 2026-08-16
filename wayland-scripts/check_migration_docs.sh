#!/bin/bash
# 检查每个迁移模块是否有独立迁移文档，并包含任务要求的六类信息。
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="${CUTEFISH_ROOT:-$(cd "$SCRIPT_DIR/.." && pwd)}"
DOC="$ROOT/doc/migration/tasks"
CODE="$ROOT/cutefish/wayland-code"

required=(
    "原项目路径"
    "迁移后路径"
    "依赖"
    "架构变化"
    "对外接口变化"
    "功能差异"
    "测试与验收"
)

expected=(
    "stage-0/compositor-core.md"
    "stage-0/shell.md"
    "stage-0/installer.md"
    "stage-0/core-package.md"
    "stage-0/core-session.md"
    "stage-0/core-settings-daemon.md"
    "stage-0/core-powerman.md"
    "stage-0/core-notificationd.md"
    "stage-0/core-polkit-agent.md"
    "stage-0/core-clipboard.md"
    "stage-0/core-cpufreq.md"
    "stage-0/core-screen-brightness.md"
    "stage-0/core-shutdown-ui.md"
    "stage-1/backends.md"
    "stage-1/wm-protocol.md"
    "stage-1/seat-input.md"
    "stage-1/xdg-activation.md"
    "stage-1/shell-core-client.md"
    "stage-1/data-device.md"
    "stage-1/shell-qml-models.md"
)

fail=0
for rel in "${expected[@]}"; do
    doc="$DOC/$rel"
    if [ ! -f "$doc" ]; then
        echo "MISSING: $doc" >&2
        fail=1
        continue
    fi
    for key in "${required[@]}"; do
        if ! grep -q "$key" "$doc"; then
            echo "MISSING-SECTION: $doc: $key" >&2
            fail=1
        fi
    done
    echo "OK: $rel"
done

# 审计迁移代码中是否残留误导性旧注释。
if grep -RniE 'Qt.X11|X11Bypass|暂时保留|fallback 到|Wayland 下兼容运行' "$CODE" \
     --include='*.cpp' --include='*.h' --include='*.qml' --include='CMakeLists.txt' \
     --include='*.xml' --include='*.service' >/tmp/cutefish-stale-comments.log 2>/dev/null; then
    echo "STALE-COMMENT-FAIL:" >&2
    cat /tmp/cutefish-stale-comments.log >&2
    fail=1
fi
rm -f /tmp/cutefish-stale-comments.log

if [ "$fail" -eq 0 ]; then
    echo "PASS: migration docs and comment hygiene checks"
else
    echo "FAIL: see above" >&2
fi
exit "$fail"
