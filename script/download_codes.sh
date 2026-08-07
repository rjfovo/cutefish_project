#!/bin/bash

script_path=`pwd`
proj_path=${script_path}/..
code_path=cutefish/code

if [ ! -d ${proj_path}/${code_path} ];then
	mkdir -p ${proj_path}/${code_path}
fi

cutefish_url=git@github.com:rjfovo
git_repos=(
	# core
	# launcher
	# dock
	# statusbar
	# screenlocker
	# daemon
	# libcutefish
	# settings
	# updator
	# fishui
	# kwin-plugins
	# sddm-theme
	# appmotor
	# qt-plugins
	# icons
	# gtk-themes
	# cursor-themes
	# filemanager
	# debinstaller
	# texteditor
	# terminal
	# screenshot
	# calculator
	# videoplayer
	# calamares
	# plymouth-theme
	# wallpapers
	# fantascene-dynamic-wallpaper
	# calamares
	# calamares-settings
	filemodel
	desktop
)

repo_len=${#git_repos[@]}
for (( i = 0; i  < ${repo_len}; i++ )); do
	if [ -d ${proj_path}/${code_path}/${git_repos[i]} ];then
		echo "${git_repos[i]} is exists"
		continue
	fi
	echo "${git_repos[i]} not exists, cloning and adding submodule..."
	cd ${proj_path}
	git submodule add ${cutefish_url}/${git_repos[i]}.git ${code_path}/${git_repos[i]}
done

app_path=app
if [ ! -d ${proj_path}/${app_path} ];then
	mkdir -p ${proj_path}/${app_path}
fi

app_repos=(
	#apppack
	#appstore
	#appstore-client
	#taskmanager
	devicemanager
)

repo_len=${#app_repos[@]}
for (( i = 0; i  < ${repo_len}; i++ )); do
	if [ -d ${proj_path}/${app_path}/${app_repos[i]} ];then
		echo "${app_repos[i]} is exists"
		continue
	fi
	echo "${app_repos[i]} not exists, cloning and adding submodule..."
	cd ${proj_path}
	git submodule add ${cutefish_url}/${app_repos[i]}.git  ${app_path}/${app_repos[i]}
done

exit 0