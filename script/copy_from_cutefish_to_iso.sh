#!/bin/bash

CURR_PATH=`pwd`
CUTEFISH_PACKAGE_PATH=${CURR_PATH}/../build_iso/package/cutefish
if [ ! -d ${CUTEFISH_PACKAGE_PATH} ]; then
    mkdir -p ${CUTEFISH_PACKAGE_PATH}
fi

rm -f ${CUTEFISH_PACKAGE_PATH}/*.deb
cp -f ${CURR_PATH}/../cutefish/output/debs/* ${CUTEFISH_PACKAGE_PATH}

exit 0