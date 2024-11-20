#!/bin/bash

CURR_PATH=`pwd`
CUTEFISH_PACKAGE_PATH=${CURR_PATH}/../build_iso/package/cutefish
if [ ! -d ${CUTEFISH_PACKAGE_PATH} ]; then
    mkdir -p ${CUTEFISH_PACKAGE_PATH}
fi


scp root@192.168.36.129:/workspace/cutefish_project/build_iso/package/cutefish/*.deb ${CUTEFISH_PACKAGE_PATH}


exit