#!/usr/bin/env bash

# Exit immediately if something fails
set -e

# Clone project if it is downloaded using curl
if [[ $(basename "$PWD") != "krunner-firefox"* ]]; then
  git clone https://github.com/alex1701c/krunner-firefox
  cd krunner-firefox/
fi

mkdir -p build
cd build
krunner_version=$(krunner --version | grep -oP "(?<=krunner )\d+")
if [[ "$krunner_version" == "6" ]]; then
    echo "Building for Plasma6"
    BUILD_QT6_OPTION="-DBUILD_WITH_QT6=ON"
else
    echo "Building for Plasma5"
    BUILD_QT6_OPTION=""
fi

cmake .. -DCMAKE_BUILD_TYPE=Release -DKDE_INSTALL_USE_QT_SYS_PATHS=ON -DBUILD_TESTING=OFF $BUILD_QT6_OPTION
make -j$(nproc)
sudo make install/fast

# KRunner needs to be restarted for the changes to be applied
if pgrep -x krunner > /dev/null
then
    kquitapp$krunner_version krunner
fi

echo "Installation finished!";
