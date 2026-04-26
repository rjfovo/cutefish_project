sddm优先读取了/usr/share/wayland-sessions/目录下的，所以我现在采用清空这个目录下的配置文件方式
但是正常操作应该优先去读取/usr/share/xsessions才对

sudo mv /usr/share/wayland-sessions/plasma.desktop /usr/share/wayland-sessions/plasma.desktop.bak