#!/bin/bash

script_path=`pwd`
cd ${script_path}/../cutefish

package_dir=output
build_dir=code
deb_dir=debs
dbgsym_dir=dbgsym
log_dir=build_log
cache_file=build.cache

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

# build commands
mk_build_deps_cmd="mk-build-deps --install \
                        --root-cmd sudo \
                        --tool \"apt-get -o Debug::pkgProblemResolver=yes  --no-install-recommends -y\" \
                        --remove"

dpkg_buildpackage_cmd="dpkg-buildpackage -b -uc -us"

function help() {
    echo "$0 {package_name}"
    echo "package list:"
    repo_len=${#git_repos[@]}
    for (( i = 0; i  < ${repo_len}; i++ )); do
	    echo ${git_repos[i]}
    done
}

function package_build() {
        local build_path=${script_path}/${build_dir}
        local code_path=${build_path}/$1
        # local build_cmd="${mk_build_deps_cmd} && ${dpkg_buildpackage_cmd}"

        echo "Code_path: "${code_path}

        # if git repo exists, make it up to date
        if [[ -d ${code_path} ]]; then
                cd ${code_path}
        else
            echo "Can't find project[${code_path}]"
            exit 0
        fi

        echo "Current dir path"
        pwd

        # build the package
        if [[ $? -eq 0 && -d ${code_path} ]]; then
                echo Start to build $2
                cd ${code_path}

                eval ${mk_build_deps_cmd} || ( echo Error: nable to solve dependencies >&2; exit 1 )
                eval ${dpkg_buildpackage_cmd}

                # if build succeed, copy the package and clean the workspace
                if [[ $? -eq 0 ]]; then
                        cd ${build_path}
                        #for dep_pkg in `find ${build_path} -regex "^.*?-build-deps.*?deb$"`; do
                        #       rm -v ${dep_pkg}
                        #done
                        for dbg_pkg in `find ${build_path} -regex "^.*?dbgsym.*?\.deb$"`; do
                                mv -v $dbg_pkg ${script_path}/${package_dir}/${dbgsym_dir}/
                        done
                        for deb_pkg in `find ${build_path} -regex "^.*?\.deb$"`; do
                                mv -v $deb_pkg ${script_path}/${package_dir}/${deb_dir}/
                        done
                        echo Package $1 built successfully
                else
                        # quit when error occured
                        echo Error: unable to build package $2 >&2
                        # build in parallel may cause strange build error on Debian 11 and Ubuntu 20.04
                        echo Try to use --noparallel option?

                        exit 1
                fi

        else
                echo Git clone $2 failed >&2
        fi
        cd ${build_path}
        mv *.changes ${script_path}/${package_dir}/${log_dir}/
        mv *.buildinfo ${script_path}/${package_dir}/${log_dir}/
        cd ${script_path}
}

function clean() {
    if [ -z "$1" ];then
        repo_len=${#git_repos[@]}
        for (( i = 0; i  < ${repo_len}; i++ )); do
            echo ${git_repos[i]}

            local build_path=${script_path}/${build_dir}
            local code_path=${build_path}/${git_repos[i]}

            if [ -d ${code_path} ];then
                cd ${code_path}
                debuild -- clean
            else 
                echo "Not found ${git_repos[i]}"
            fi
        done
    else
        local build_path=${script_path}/${build_dir}
        local code_path=${build_path}/$1

        cd ${code_path}
        if [ -d ${code_path} ];then
            cd ${code_path}
            debuild -- clean
        else 
            echo "Not found $1"
        fi
    fi
}

if [[ ! -d ${package_dir} ]]; then
        mkdir ${package_dir}
fi

cd ${package_dir}
[[ -e ${cache_file} ]] || touch ${cache_file}
[[ -d ${build_dir} ]] || mkdir ${build_dir}
[[ -d ${deb_dir} ]] || mkdir ${deb_dir}
[[ -d ${dbgsym_dir} ]] || mkdir ${dbgsym_dir}

if [ $# -lt 1 ];then
    help
    exit 0
fi

if [ $1 == "all" ];then
    # 编译所有软件包
    repo_len=${#git_repos[@]}
    for (( i = 0; i  < ${repo_len}; i++ )); do
            package_build ${git_repos[i]}
    done
elif [ $1 == "clean" ];then
    echo test
    clean $2
else
    # 单独编译软件包
    package_build ${1}
fi

exit 0