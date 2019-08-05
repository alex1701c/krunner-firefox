#!/bin/bash

# Exit immediately if something goes wrong
set -e

cp /usr/share/applications/firefox.desktop ~/.local/share/applications/firefox.desktop

if [[ $(basename "$PWD") != "FirefoxProfileRunner"* ]];then
    git clone https://github.com/alex1701c/FirefoxProfileRunner
    cd FirefoxProfileRunner/
fi

mkdir -p build
cd build
cmake -DQT_PLUGIN_INSTALL_DIR=`kf5-config --qt-plugins` ..
make -j2
sudo make install

kquitapp5 krunner 2> /dev/null
kstart5 --windowclass krunner krunner > /dev/null 2>&1 &

echo "Installation finished !";
