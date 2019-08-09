#!/bin/bash

# Extract and move icon
mkdir -p /tmp/firefoxprofile_installer
unzip /usr/lib/firefox/browser/omni.ja -c "chrome/browser/skin/classic/browser/privatebrowsing/favicon.svg" -d /tmp/firefoxprofile_installer > /dev/null 2>&1
sudo mv /tmp/firefoxprofile_installer/chrome/browser/skin/classic/browser/privatebrowsing/favicon.svg /usr/share/icons/private_browsing_firefox.svg
rm -rf /tmp/firefoxprofile_installer

# Exit immediately if something goes wrong
set -e

# This plugin uses a local copy of the firefox desktop file to avoid permission issues
mkdir -p ~/.local/share/applications
cp /usr/share/applications/firefox.desktop ~/.local/share/applications/firefox.desktop

# Clone project if it is downloaded using curl
if [[ $(basename "$PWD") != "FirefoxProfileRunner"* ]]; then
  git clone https://github.com/alex1701c/FirefoxProfileRunner
  cd FirefoxProfileRunner/
fi

mkdir -p build
cd build
cmake -DQT_PLUGIN_INSTALL_DIR=$(kf5-config --qt-plugins) ..
make -j2
sudo make install

# Restart Krunner
kquitapp5 krunner 2>/dev/null
kstart5 --windowclass krunner krunner >/dev/null 2>&1 &

echo "Installation finished !"
