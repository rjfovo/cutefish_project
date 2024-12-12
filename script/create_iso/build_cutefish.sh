#!/bin/bash

source ./env.sh

echo $PWD
echo $LIVE_BOOT
echo $DEBIAN_CHROOT
echo $BUILD_CONFIG
echo $BUILD_SCRIPT

if [ -z "$LIVE_BOOT" ];then
    echo "Empty LIVE_BOOT path $LIVE_BOOT"
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
    echo "$LIVE_BOOT 已经目录存在，正在删除..."
    rm -rf $LIVE_BOOT/../LIVE_BOOT/*
    echo "目录已删除"
}

if [ "$1" == "--clean" ];then
    clean
    exit 0;
elif [ "$1" == "--rebuild" ];then
    # 删除已经存在的workspace目录
    clean
elif [ "$1" == "--build" ];then
    echo "start build"
else
    help
    exit 0
fi

./build_live_filesystem.sh
./build_install_filesystem.sh

if [ -d $LIVE_BOOT/live_chroot ];then
    ./create_iso.sh
else 
    echo "Not found $LIVE_BOOT/live_chroot build iso failed!!!"
fi

exit