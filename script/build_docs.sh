#!/bin/bash

script_path=`pwd`
script_path=${script_path}/../cutefish
cd ${script_path}/code/docs

/root/.local/pipx/venvs/sphinx-rtd-theme/bin/sphinx-build -M html ./source/ /var/www/myos/

exit 0