#!/bin/bash

source env.sh
echo ${DEBIAN_LIVE_CHROOT}
ls ${BUILD_CONFIG}
ls ${BUILD_SCRIPT}

mkdir -p ${DEBIAN_LIVE_CHROOT}
if [ -f ${DEBIAN_SOURCE}/chroot.tar.gz ];then
    tar -xzvf ${DEBIAN_SOURCE}/chroot.tar.gz -C ${DEBIAN_LIVE_CHROOT}

    BUILD_PATH=`pwd`
    cd ${DEBIAN_LIVE_CHROOT}
    mv ${DEBIAN_LIVE_CHROOT}/chroot/* ./
    rm -r chroot
    cd ${BUILD_PATH}
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
mount --bind /dev/pts ${DEBIAN_LIVE_CHROOT}/dev/pts
mount -t proc proc ${DEBIAN_LIVE_CHROOT}/proc
mount -t sysfs sysfs ${DEBIAN_LIVE_CHROOT}/sys
mount --bind /run ${DEBIAN_LIVE_CHROOT}/run

cp /etc/resolv.conf ${DEBIAN_LIVE_CHROOT}/etc/resolv.conf

echo "cutefish-live" | sudo tee "${DEBIAN_LIVE_CHROOT}/etc/hostname"
# 配置live环境
sudo chroot "${DEBIAN_LIVE_CHROOT}" << EOF
    apt-get update 

    # 移除可能存在的 grub-pc 以避免与 grub-efi-amd64 冲突
    apt-get remove -y --purge grub-pc 2>/dev/null || true
    # 也移除 grub2 虚拟包，因为它依赖 grub-pc
    apt-get remove -y --purge grub2 2>/dev/null || true

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
        grub-common \
        grub2-common \
        grub-efi-amd64 \
        grub-efi-amd64-bin \
        grub-efi-amd64-signed \
        grub-efi-ia32-bin \
        grub-pc-bin \
        python3 \
        dialog \
        locales \
        ssh \
        rsync 
EOF

# 配置字体
sudo chroot "${DEBIAN_LIVE_CHROOT}" << EOF
    apt-get install -y --no-install-recommends \
        xfonts-utils \
        fontconfig \
        fonts-noto-cjk

    apt install  -y --no-install-recommends \
            locales locales-all \
            fontconfig \
            fonts-dejavu-core \
            fonts-noto-core \
            fonts-noto-cjk \
            fonts-noto-color-emoji

    apt install -y fonts-wqy-zenhei
    fc-cache -fv

    echo "en_US.UTF-8 UTF-8" >> /etc/locale.gen
    echo 'LANG=en_US.UTF-8' > /etc/environment

    locale
    locale -a
    fc-list | head
EOF

# 配置显示相关软件包
sudo chroot "${DEBIAN_LIVE_CHROOT}" << EOF
    apt-get install -y --no-install-recommends\
        xserver-xorg-core xserver-xorg xinit xterm 
EOF

# 配置pokit相关软件包
sudo chroot "${DEBIAN_LIVE_CHROOT}" << EOF
    apt-get install -y --no-install-recommends\
        polkitd pkexec
EOF

# 创建文件系统需要的软件包
sudo chroot "${DEBIAN_LIVE_CHROOT}" << EOF
    apt-get install -y --no-install-recommends\
        dosfstools \
        e2fsprogs
EOF

# # 安装cutefish安装器
BUILD_OLD_DIR=`pwd`
# 在build系统中构建cutefish apt 软件包
./build_apt.sh --rebuild

# 启动http服务用于软件包访问
killall -9 python3
cd ${CUTEFISH_APT_DIR}
python3 -m http.server 8080 &
sleep 10 # 等待http服务启动

# 安装所有cutefish软件
sudo chroot "${DEBIAN_LIVE_CHROOT}" << EOF
    echo "deb [trusted=yes] http://192.168.118.129:8080 stable non-free" > /etc/apt/sources.list.d/cutefish.list
    apt clean
    apt update

    apt install -y --no-install-recommends appmotor
    apt install -y --no-install-recommends cutefish-calculator
    apt install -y --no-install-recommends cutefish-calamares
    apt install -y --no-install-recommends cutefish-core
    apt install -y --no-install-recommends cutefish-cursor-themes
    apt install -y --no-install-recommends cutefish-daemon
    apt install -y --no-install-recommends cutefish-debinstaller
    apt install -y --no-install-recommends cutefish-dock
    apt install -y --no-install-recommends cutefish-filemanager
    apt install -y --no-install-recommends cutefish-gtk-themes
    apt install -y --no-install-recommends cutefish-icons
    apt install -y --no-install-recommends cutefish-kwin-plugins
    apt install -y --no-install-recommends cutefish-launcher
    apt install -y --no-install-recommends cutefish-plymouth-theme
    apt install -y --no-install-recommends cutefish-qt-plugins
    apt install -y --no-install-recommends cutefish-screenlocker
    apt install -y --no-install-recommends cutefish-screenshot
    apt install -y --no-install-recommends cutefish-sddm-theme
    apt install -y --no-install-recommends cutefish-settings
    apt install -y --no-install-recommends cutefish-statusbar
    apt install -y --no-install-recommends cutefish-terminal
    apt install -y --no-install-recommends cutefish-updator
    apt install -y --no-install-recommends cutefish-videoplayer
    apt install -y --no-install-recommends cutefish-wallpapers
    apt install -y --no-install-recommends fishui
    apt install -y --no-install-recommends libcutefish
    apt install -y --no-install-recommends texteditor
    apt install -y --no-install-recommends yoyo-fantacy
EOF

cd ${BUILD_OLD_DIR}

cp ${BUILD_SCRIPT}/user/myuser ${DEBIAN_LIVE_CHROOT}/usr/bin
cp ${BUILD_SCRIPT}/live/cutefish_installer ${DEBIAN_LIVE_CHROOT}/usr/bin

cp ${BUILD_CONFIG}/sddm_autologin.conf ${DEBIAN_LIVE_CHROOT}/etc/sddm.conf.d/autologin.conf      # sddm用户自动登录
cp ${BUILD_CONFIG}/cutefish-installer.desktop ${DEBIAN_LIVE_CHROOT}/etc/xdg/autostart            # 添加进入桌面后自动启动程序
# 修改配置文件，不然calamares执行分区时会卡死，这里进行特殊处理
cp -f ${BUILD_CONFIG}/org.kde.kpmcore.helperinterface.conf ${DEBIAN_LIVE_CHROOT}/usr/share/dbus-1/system.d/org.kde.kpmcore.helperinterface.conf

# 创建live用户以及设置免密登录
sudo chroot "${DEBIAN_LIVE_CHROOT}" << EOF
    chmod 777 /usr/bin/myuser
    chmod 777 /usr/bin/cutefish_installer

    myuser --add cutefish-live cutefish
    echo "cutefish-live ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers
    usermod -aG sudo cutefish-live

    echo "export PATH=/usr/sbin:\$PATH" >> /home/cutefish-live/user/.bashrc
    cat /home/cutefish-live/user/.bashrc

    echo "127.0.0.1   cutefish-live" >> /etc/hosts
EOF


umount ${DEBIAN_LIVE_CHROOT}/dev/pts
umount ${DEBIAN_LIVE_CHROOT}/dev
umount ${DEBIAN_LIVE_CHROOT}/proc
umount ${DEBIAN_LIVE_CHROOT}/sys
umount ${DEBIAN_LIVE_CHROOT}/run

