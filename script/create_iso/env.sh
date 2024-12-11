#!/bin/bash

PWD=`pwd`

#LIVE_BOOT=${PWD}/LIVE_BOOT
LIVE_BOOT=/mnt/disk1/LIVE_BOOT
DEBIAN_LIVE_CHROOT=${LIVE_BOOT}/live_chroot
DEBIAN_INSTALL_CHROOT=${LIVE_BOOT}/install_chroot

BUILD_CONFIG=../../build_iso/config
BUILD_SCRIPT=../../build_iso/script
BUILD_PACKAGE=../../build_iso/package

DEBIAN_SOURCE=/workspace/debian-sources/