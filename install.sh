#!/bin/bash

# Extract and move icon
mkdir -p /tmp/firefoxprofile_installer
if [[ -e /usr/lib/firefox/browser/omni.ja ]]; then
    unzip /usr/lib/firefox/browser/omni.ja -c "chrome/browser/skin/classic/browser/privatebrowsing/favicon.svg" -d /tmp/firefoxprofile_installer > /dev/null 2>&1
else
    unzip /usr/lib/firefox-esr/browser/omni.ja -c "chrome/browser/skin/classic/browser/privatebrowsing/favicon.svg" -d /tmp/firefoxprofile_installer > /dev/null 2>&1
    # Very dirty workaround but this way you can install it without manual modifications
    echo "[Desktop Entry]
Name=FirefoxProfileRunner
Comment=Krunner plugin to launch Firefox profiles
Icon=firefox-esr

X-KDE-ServiceTypes=Plasma/Runner
Type=Service
X-KDE-Library=krunner_firefoxprofilerunner
X-KDE-PluginInfo-Author=Alex1701c
X-KDE-PluginInfo-Email=alex1701c.dev@gmx.net
X-KDE-PluginInfo-Name=firefoxprofilerunner
X-KDE-PluginInfo-Version=1.2.4
X-KDE-PluginInfo-License=GPL 3
X-KDE-PluginInfo-EnabledByDefault=true" > src/plasma-runner-firefoxprofilerunner.desktop
fi

# Exit immediately if something goes wrong
set -e

sudo mv /tmp/firefoxprofile_installer/chrome/browser/skin/classic/browser/privatebrowsing/favicon.svg /usr/share/icons/private_browsing_firefox.svg
rm -rf /tmp/firefoxprofile_installer

# This plugin uses a local copy of the firefox desktop file to avoid permission issues
mkdir -p ~/.local/share/applications
cp /usr/share/applications/firefox*.desktop ~/.local/share/applications/

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
