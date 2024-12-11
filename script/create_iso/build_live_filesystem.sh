#!/bin/bash

source env.sh
echo ${DEBIAN_LIVE_CHROOT}
ls ${BUILD_CONFIG}
ls ${BUILD_SCRIPT}

mkdir -p ${DEBIAN_LIVE_CHROOT}
if [ -f ${DEBIAN_SOURCE}/chroot.tar.gz ];then
    tar -xzvf ${DEBIAN_SOURCE}/chroot.tar.gz -C ${DEBIAN_LIVE_CHROOT}

    PWD=`pwd`
    cd ${DEBIAN_LIVE_CHROOT}
    mv ${DEBIAN_LIVE_CHROOT}/chroot/* ./
    rm -r chroot
    cd ${PWD}
else
    # 下载debian base
    sudo debootstrap \
        --arch=amd64 \
        --variant=minbase \
        stable \
        "${DEBIAN_LIVE_CHROOT}" \
        https://mirrors.aliyun.com/debian/
fi

mount --bind /dev ${DEBIAN_LIVE_CHROOT}/dev
mount -t proc proc ${DEBIAN_LIVE_CHROOT}/proc
mount -t sysfs sysfs ${DEBIAN_LIVE_CHROOT}/sys

echo "cutefish-live" | sudo tee "${DEBIAN_LIVE_CHROOT}/etc/hostname"
# 配置live环境
sudo chroot "${DEBIAN_LIVE_CHROOT}" << EOF
apt-get update && \
apt-get install -y --no-install-recommends \
    linux-image-amd64 \
    live-boot \
    systemd-sysv \
    sudo \
    iproute2 \
    dbus \
    network-manager \
    vim \
    squashfs-tools \
    grub2
EOF

sudo chroot "${DEBIAN_LIVE_CHROOT}" << EOF
apt-get install -y --no-install-recommends \
    xserver-xorg-core xserver-xorg xinit xterm 
EOF

# # 安装cutefish安装器
mkdir ${DEBIAN_LIVE_CHROOT}/package
cp ${BUILD_PACKAGE}/cutefish/*.deb ${DEBIAN_LIVE_CHROOT}/package/

# 安装所有cutefish软件
sudo chroot "${DEBIAN_LIVE_CHROOT}" << EOF
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
rm -rf ${DEBIAN_LIVE_CHROOT}/package

cp ${BUILD_SCRIPT}/user/myuser ${DEBIAN_LIVE_CHROOT}/usr/bin
cp ${BUILD_SCRIPT}/live/cutefish_installer ${DEBIAN_LIVE_CHROOT}/usr/bin

cp ${BUILD_CONFIG}/sddm_autologin.conf ${DEBIAN_LIVE_CHROOT}/etc/sddm.conf.d/autologin.conf      # sddm用户自动登录
cp ${BUILD_CONFIG}/cutefish-installer.desktop ${DEBIAN_LIVE_CHROOT}/etc/xdg/autostart            # 添加进入桌面后自动启动程序

# 创建live用户以及设置免密登录
sudo chroot "${DEBIAN_LIVE_CHROOT}" << EOF
    chmod 777 /usr/bin/myuser
    chmod 777 /usr/bin/cutefish_installer

    myuser --add cutefish-live cutefish
    echo "cutefish-live ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers
    echo "export PATH=/usr/sbin:\$PATH" >> /home/cutefish-live/user/.bashrc
    cat /home/cutefish-live/user/.bashrc
EOF

umount ${DEBIAN_LIVE_CHROOT}/dev
umount ${DEBIAN_LIVE_CHROOT}/proc
umount ${DEBIAN_LIVE_CHROOT}/sys

