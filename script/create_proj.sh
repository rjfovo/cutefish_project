#!/bin/bash

CURR_PATH=`pwd`

function build_code()
{
    code_project_path=${CURR_PATH}/../cutefish
    if [ -d ${code_project_path} ];then
        echo "${code_project_path} exists"
        exit 0
    fi

    mkdir -p ${code_project_path}

    exit 0
}

function build_iso()
{
    code_project_path=${CURR_PATH}/../build_iso
    if [ -d ${code_project_path} ];then
        echo "${code_project_path} exists"
        exit 0
    fi

    mkdir -p ${code_project_path}
    mkdir -p ${code_project_path}/package
    mkdir -p ${code_project_path}/package/cutefish

    cp -f ${CURR_PATH}/../cutefish/output/debs/* ${code_project_path}/package/cutefish

    exit 0
}

if [ "$1" == "code" ]; then
    echo "build code "
    build_code
elif [ "$1" == "build_iso" ]; then
    echo "build iso"
    build_iso
else
    echo "Unknow option"
fi