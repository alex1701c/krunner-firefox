# Firefox Profile Runner

This plugin allows you to launch Firefox profiles from Krunner and your normal launcher.

The plugin uses a local copy of the firefox.desktop file. In this file it adds Desktop Action entries for
the profiles. Because of this you can search for the profiles in your normal launcher.  
This plugin is also compatible with firefox-esr.

Furthermore it provides proxychains4 integration. This allows you to launch specific profiles over a proxy.
This integration is optional.

You can find deb/rpm packages on the release page.

### Required Dependencies

Debian/Ubuntu:

    sudo apt install cmake extra-cmake-modules build-essential libkf5runner-dev libkf5textwidgets-dev qtdeclarative5-dev gettext libkf5kcmutils-dev

openSUSE:

    sudo zypper install cmake extra-cmake-modules libQt5Widgets5 libQt5Core5 libqt5-qtlocation-devel ki18n-devel ktextwidgets-devel kservice-devel krunner-devel gettext-tools kconfigwidgets-devel kcmutils-devel  

Fedora:

    sudo dnf install cmake extra-cmake-modules kf5-ki18n-devel kf5-kservice-devel kf5-krunner-devel kf5-ktextwidgets-devel gettext kf5-kcmutils-devel  

Arch/Manjaro:
    sudo pacman -S cmake extra-cmake-modules kcmutils

### Quick Install

Download the latest install script:

    curl https://raw.githubusercontent.com/alex1701c/krunner-firefox/master/install.sh

To install as a global plugin:

    bash install.sh

To install as a user plugin, admin privileges not needed:

    GLOBAL_PLUGIN=false bash ./install.sh

The install script can also be executed from the cloned repository:

    git clone https://github.com/alex1701c/krunner-firefox
    cd krunner-firefox
    GLOBAL_PLUGIN=true bash ./install.sh

### Manual build and install instructions

To install as a global plugin:

    git clone https://github.com/alex1701c/krunner-firefox
    cd krunner-firefox
    mkdir build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Release -DKDE_INSTALL_QTPLUGINDIR=$(kf5-config --qt-plugins) ..
    make
    sudo make -j$(nproc) install
    kquitapp5 krunner 2> /dev/null; kstart5 --windowclass krunner krunner > /dev/null 2>&1 &

To get the icon for the private window you have to extract it:

    mkdir -p /tmp/firefoxprofile_installer
    unzip /usr/lib/firefox/browser/omni.ja -c "chrome/browser/skin/classic/browser/privatebrowsing/favicon.svg" -d /tmp/firefoxprofile_installer > /dev/null 2>&1
    sudo mv /tmp/firefoxprofile_installer/chrome/browser/skin/classic/browser/privatebrowsing/favicon.svg /usr/share/pixmaps/private_browsing_firefox.svg
    rm -rf /tmp/firefoxprofile_installer

If you install it manually for firefox-esr you should adjust the icon in src/plasma-runner-firefoxprofilerunner.desktop
and change the path for the unzip command.

## Screenshots

#### Overview

The plugin gets triggered by typing "firef" in KRunner:

![Overview](https://raw.githubusercontent.com/alex1701c/Screenshots/master/FirefoxProfileRunner/runner_profiles_overview.png)

#### Search

After the trigger word you can search the profiles:

![Search](https://raw.githubusercontent.com/alex1701c/Screenshots/master/FirefoxProfileRunner/runner_profiles_search.png)

#### Config Dialog

If you uncheck the options to register the normal/private window launch options profiles globally you can not launch
the profiles like in the last screenshot.

If the following option is checked the option to launch a normal window with the default profile is not shown.
This is useful, because the default profile can be launched with the "Applications" plugin.

The option to always show private windows is demonstrated in the first screenshot.

Using the Move Up/Down buttons you can change the order of the options.

The text field below is used to rename the profiles. This does not modify the Firefox
configuration, just the displayed name for the runner.

![Config Dialog](https://raw.githubusercontent.com/alex1701c/Screenshots/master/FirefoxProfileRunner/general_config_dialog.png)

#### Config Dialog Proxychains

If proxychains4 is installed you can add extra launch options. This way Firefox is started over a proxy
but no system wide proxy has to be configured.

The force new instance button adds the "--new-instance" flag to the launch arguments of Firefox. This way
you can be sure that Firefox is started over a proxy, but you can only have one instance of Firefox with
the selected profile running.

![Config Dialog Proxychains](https://raw.githubusercontent.com/alex1701c/Screenshots/master/FirefoxProfileRunner/config_proxychains_extra.png)

This configuration would look like this in Krunner:

![Proxychains Config Example](https://raw.githubusercontent.com/alex1701c/Screenshots/master/FirefoxProfileRunner/runner_profiles_search_proxychains.png)

The existing launch options can also be modified to use proxychains.

![Config Dialog Proxychains Change Existing](https://raw.githubusercontent.com/alex1701c/Screenshots/master/FirefoxProfileRunner/config_proxychains_existing.png)

#### Overview With All Global Options

![Overview With All Global Options](https://raw.githubusercontent.com/alex1701c/Screenshots/master/FirefoxProfileRunner/global_overview_proxychains.png)
