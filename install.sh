#!/bin/bash

# Extract and move icon
mkdir -p /tmp/firefoxprofile_installer
if [[ -e /usr/lib/firefox/browser/omni.ja ]]; then
    unzip /usr/lib/firefox/browser/omni.ja -c "chrome/browser/skin/classic/browser/privatebrowsing/favicon.svg" -d /tmp/firefoxprofile_installer > /dev/null 2>&1
else
    unzip /usr/lib/firefox-esr/browser/omni.ja -c "chrome/browser/skin/classic/browser/privatebrowsing/favicon.svg" -d /tmp/firefoxprofile_installer > /dev/null 2>&1
    sed -i "s/Icon=.*/Icon=firefox-esr/" src/plasma-runner-firefoxprofilerunner.desktop
fi

# Exit immediately if something goes wrong
set -e

sudo mkdir -p /usr/share/pixmaps/
sudo mv /tmp/firefoxprofile_installer/chrome/browser/skin/classic/browser/privatebrowsing/favicon.svg /usr/share/pixmaps/private_browsing_firefox.svg
rm -rf /tmp/firefoxprofile_installer

# Clone project if it is downloaded using curl
if [[ $(basename "$PWD") != "krunner-firefox"* ]]; then
  git clone https://github.com/alex1701c/krunner-firefox
  cd krunner-firefox/
fi

mkdir -p build
cd build

cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
sudo make install

# Restart Krunner
kquitapp5 krunner 2>/dev/null
kstart5 --windowclass krunner krunner >/dev/null 2>&1 &

echo "Installation finished !"
