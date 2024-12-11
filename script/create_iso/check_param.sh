#!/bin/bash

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
    #rm -rf $LIVE_BOOT/../LIVE_BOOT/*
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