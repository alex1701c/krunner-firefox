#!/bin/bash

# Extract and move icon
mkdir -p ~/tmp/firefoxprofile_installer
if [[ -e /usr/lib/firefox/browser/omni.ja ]]; then
    unzip /usr/lib/firefox/browser/omni.ja -c "chrome/browser/skin/classic/browser/privatebrowsing/favicon.svg" -d ~/tmp/firefoxprofile_installer > /dev/null 2>&1
else
    unzip /usr/lib/firefox-esr/browser/omni.ja -c "chrome/browser/skin/classic/browser/privatebrowsing/favicon.svg" -d ~/tmp/firefoxprofile_installer > /dev/null 2>&1
    # Very dirty workaround but this way you can install it without manual modifications
    sed -i "s/Icon=.*/Icon=firefox-esr/" src/plasma-runner-firefoxprofilerunner.desktop
fi

# Exit immediately if something goes wrong
set -e

mkdir -p ~/.local/share/pixmaps/
mv ~/tmp/firefoxprofile_installer/chrome/browser/skin/classic/browser/privatebrowsing/favicon.svg ~/.local/share/pixmaps/private_browsing_firefox.svg
rm -rf ~/tmp/firefoxprofile_installer

# This plugin uses a local copy of the firefox desktop file to avoid permission issues
mkdir -p ~/.local/share/applications
cp /usr/share/applications/firefox*.desktop ~/.local/share/applications/

# Clone project if it is downloaded using curl
if [[ $(basename "$PWD") != "krunner-firefox"* ]]; then
  git clone https://github.com/alex1701c/krunner-firefox
  cd krunner-firefox/
fi

# Create folders
mkdir -p build
mkdir -p ~/.local/lib/qt/plugins
mkdir -p ~/.local/share/kservices5

cd build

# Add the installation path to the QT_PLUGIN_PATH
if [[ -z "${QT_PLUGIN_PATH}" || "${QT_PLUGIN_PATH}" != *".local/lib/qt/plugins/"* ]]; then
    echo "export QT_PLUGIN_PATH=~/.local/lib/qt/plugins/:$QT_PLUGIN_PATH" >> ~/.bashrc
    export QT_PLUGIN_PATH=~/.local/lib/qt/plugins/:$QT_PLUGIN_PATH
fi

cmake -DQT_PLUGIN_INSTALL_DIR="~/.local/lib/qt/plugins" -DKDE_INSTALL_KSERVICES5DIR="~/.local/share/kservices5" -DCMAKE_BUILD_TYPE=Release  ..
make -j$(nproc)
make install

kquitapp5 krunner 2> /dev/null
kstart5 --windowclass krunner krunner > /dev/null 2>&1 &

echo "Installation finished !";
