#!/bin/bash

source ./env.sh

echo $PWD
echo $CUTEFISH_APT_DIR

if [ -z "$CUTEFISH_APT_DIR" ];then
    echo "Empty CUTEFISH_APT_DIR path $CUTEFISH_APT_DIR"
    exit 0
fi

function help()
{
    echo "option: "
    echo "  --clean"
    echo "  --rebuild"
    echo "  --build"
}

function clean()
{
    echo "$CUTEFISH_APT_DIR 已经目录存在，正在删除..."
    rm -rf $CUTEFISH_APT_DIR
    echo "目录已删除"
}

if [ "$1" == "--clean" ];then
    clean
    exit 0;
elif [ "$1" == "--rebuild" ];then
    # 删除已经存在的workspace目录
    clean
    mkdir -p $CUTEFISH_APT_DIR
elif [ "$1" == "--build" ];then
    echo "start build"
else
    help
    exit 0
fi

# 仓库目录架构
# repo-root/
# ├── pool/
# │   └── main/
# │       └── d/
# │           └── dsz/
# │               └── dsz_1.0.0_all.deb
# └── dists/
#     └── stable/
#         └── main/
#             └── binary-amd64/
#                 ├── Packages
#                 └── Packages.gz

mkdir -p ${CUTEFISH_APT_DIR}/pool
mkdir -p ${CUTEFISH_APT_PACKAGE_DIR}
mkdir -p ${CUTEFISH_APT_INDEX_DIR}

# 复制软件包到repo中
for DEB_NAME in `ls ${BUILD_PACKAGE}/cutefish`
do
    if [[ "${DEB_NAME}" == *.deb ]]; then
        echo "Is deb ${DEB_NAME}"
    else
        echo "Not deb ${DEB_NAME}"
        continue
    fi

    DEB_FIRST_CHAR="${DEB_NAME:0:1}"
    PACKAGE_NAME=$(echo "${DEB_NAME}" | awk -F_ '{print $1}')
    echo ${DEB_NAME} ${DEB_FIRST_CHAR} ${PACKAGE_NAME}

    mkdir -p ${CUTEFISH_APT_PACKAGE_DIR}/${DEB_FIRST_CHAR}/${PACKAGE_NAME}
    cp ${BUILD_PACKAGE}/cutefish/${DEB_NAME} ${CUTEFISH_APT_PACKAGE_DIR}/${DEB_FIRST_CHAR}/${PACKAGE_NAME}
done

# 生成索引文件
cd ${CUTEFISH_APT_DIR}
apt-ftparchive packages . > ${CUTEFISH_APT_INDEX_DIR}/Packages
gzip -9c ${CUTEFISH_APT_INDEX_DIR}/Packages > ${CUTEFISH_APT_INDEX_DIR}/Packages.gz

exit 0