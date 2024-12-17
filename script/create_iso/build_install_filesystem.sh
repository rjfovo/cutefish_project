#!/bin/bash

source env.sh
echo ${DEBIAN_INSTALL_CHROOT}
echo ${DEBIAN_LIVE_CHROOT}
ls ${BUILD_CONFIG}
ls ${BUILD_SCRIPT}

mkdir -p ${DEBIAN_INSTALL_CHROOT}
if [ -f ${DEBIAN_SOURCE}/chroot.tar.gz ];then
    tar -xzvf ${DEBIAN_SOURCE}/chroot.tar.gz -C ${DEBIAN_INSTALL_CHROOT}
    
    BUILD_PATH=`pwd`
    cd ${DEBIAN_INSTALL_CHROOT}
    mv ${DEBIAN_INSTALL_CHROOT}/chroot/* ./
    rm -r chroot
    cd ${BUILD_PATH}
else
    # 下载debian base
    sudo debootstrap \
        --arch=amd64 \
        --variant=minbase \
        stable \
        "${DEBIAN_INSTALL_CHROOT}" \
        https://mirrors.aliyun.com/debian/
fi

mount --bind /dev ${DEBIAN_INSTALL_CHROOT}/dev
mount -t proc proc ${DEBIAN_INSTALL_CHROOT}/proc
mount -t sysfs sysfs ${DEBIAN_INSTALL_CHROOT}/sys

echo "cutefish-live" | sudo tee "${DEBIAN_INSTALL_CHROOT}/etc/hostname"
# 配置live环境
sudo chroot "${DEBIAN_INSTALL_CHROOT}" << EOF
apt-get update && \
apt-get install -y --no-install-recommends \
    linux-image-amd64 \
    systemd-sysv \
    live-boot \
    sudo \
    iproute2 \
    dbus \
    network-manager \
    vim \
    grub2
EOF

sudo chroot "${DEBIAN_INSTALL_CHROOT}" << EOF
apt-get install -y --no-install-recommends \
    xserver-xorg-core xserver-xorg xinit
EOF

# # 安装cutefish安装器
pwd
mkdir ${DEBIAN_INSTALL_CHROOT}/package
cp ${BUILD_PACKAGE}/cutefish/*.deb ${DEBIAN_INSTALL_CHROOT}/package/
rm -f ${DEBIAN_INSTALL_CHROOT}/package/cutefish-calamares_0.5_amd64.deb # 安装的系统不需要calamare包

# 安装所有cutefish软件
sudo chroot "${DEBIAN_INSTALL_CHROOT}" << EOF
    cd /package
    dpkg -i *.deb
    rm -f /var/cache/apt/archives/*
    apt --fix-broken -d install -y
    cd /var/cache/apt/archives/
    dpkg -i --force-overwrite *.deb
    apt --fix-broken install -y
    cd /package
    dpkg -i --force-overwrite *.deb
    apt remove kdeconnect -y
    apt remove zutty -y 
    apt remove plasma-discover -y
    apt remove systemsettings -y
    apt remove plasma-systemmonitor -y
    apt remove partitionmanager -y
    apt remove kwalletmanager -y
    apt remove plasma-workspace -y
    
    apt autoremove -y
    rm -f /var/cache/apt/archives/*
EOF
rm -rf ${DEBIAN_INSTALL_CHROOT}/package

umount ${DEBIAN_INSTALL_CHROOT}/dev
umount ${DEBIAN_INSTALL_CHROOT}/proc
umount ${DEBIAN_INSTALL_CHROOT}/sys
