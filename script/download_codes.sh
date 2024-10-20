#!/bin/bash

script_path=`pwd`
code_path=${script_path}/../cutefish/code

if [ ! -d ${code_path} ];then
	mkdir -p ${code_path}
fi
cd ${code_path}

cutefish_url=https://github.com/rjfovo
git_repos=(
	core
	launcher
	dock
	statusbar
	screenlocker
	daemon
	libcutefish
	settings
	updator
	fishui
	kwin-plugins
	sddm-theme
	appmotor
	qt-plugins
	icons
	gtk-themes
	cursor-themes
	filemanager
	debinstaller
	texteditor
	terminal
	screenshot
	calculator
	videoplayer
	calamares
	plymouth-theme
	wallpapers
)

repo_len=${#git_repos[@]}
for (( i = 0; i  < ${repo_len}; i++ )); do
	git clone ${cutefish_url}/${git_repos[i]}.git
done

exit 0