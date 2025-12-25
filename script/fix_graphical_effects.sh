#!/bin/bash
# 批量修改 QML 文件中的 GraphicalEffects 导入
# 将 import Qt5Compat.GraphicalEffects 1.0 替换为 import Qt5Compat.GraphicalEffects 6.0

set -e

cd /workspace/cutefish_project

# 查找所有包含 import Qt5Compat.GraphicalEffects 1.0 的 QML 文件
files=$(grep -r "import Qt5Compat.GraphicalEffects 1.0" cutefish/code --include="*.qml" | cut -d: -f1 | sort -u)

if [ -z "$files" ]; then
    echo "没有找到需要修改的文件。"
    exit 0
fi

echo "找到以下文件需要修改："
echo "$files"
echo ""

for file in $files; do
    echo "正在修改 $file"
    # 使用 sed 进行替换
    sed -i 's/import Qt5Compat.GraphicalEffects 1.0/import Qt5Compat.GraphicalEffects 6.0/g' "$file"
done

echo "修改完成。"
