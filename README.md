# Firefox Profile Runner

This plugin allows you to launch Firefox profiles from Krunner and your normal launcher.

The plugin uses a local copy of the firefox.desktop file. In this file it adds Desktop Action entries for
the profiles. Because of this you can search for the profiles in your normal launcher.  
If you make changes in your Firefox profile you can sync them manually in the config dialog or they will
be applied on startup. 

This plugin is also compatible with firefox-esr.
 
### Required Dependencies

Debian/Ubuntu  
`sudo apt install cmake extra-cmake-modules build-essential libkf5runner-dev libkf5textwidgets-dev qtdeclarative5-dev gettext`

openSUSE  
`sudo zypper install cmake extra-cmake-modules libQt5Widgets5 libQt5Core5 libqt5-qtlocation-devel ki18n-devel ktextwidgets-devel kservice-devel krunner-devel gettext-tools`  

Fedora  
`sudo dnf install cmake extra-cmake-modules kf5-ki18n-devel kf5-kservice-devel kf5-krunner-devel kf5-ktextwidgets-devel gettext`  

### Build instructions


The easiest way to install is:  
`curl https://raw.githubusercontent.com/alex1701c/FirefoxProfileRunner/master/install.sh | bash`

Or you can do it manually:  
```
mkdir -p ~/.local/share/applications
cp /usr/share/applications/firefox*.desktop ~/.local/share/applications/
git clone https://github.com/alex1701c/FirefoxProfileRunner
cd FirefoxProfileRunner
mkdir build
cd build
cmake -DQT_PLUGIN_INSTALL_DIR=`kf5-config --qt-plugins` ..
make
sudo make install
kquitapp5 krunner 2> /dev/null; kstart5 --windowclass krunner krunner > /dev/null 2>&1 &
```

To get the icon for the private window you have to extract it:   
```
mkdir -p /tmp/firefoxprofile_installer
unzip /usr/lib/firefox/browser/omni.ja -c "chrome/browser/skin/classic/browser/privatebrowsing/favicon.svg" -d /tmp/firefoxprofile_installer > /dev/null 2>&1
sudo mv /tmp/firefoxprofile_installer/chrome/browser/skin/classic/browser/privatebrowsing/favicon.svg /usr/share/icons/private_browsing_firefox.svg
rm -rf /tmp/firefoxprofile_installer
```
If you install it manually for firefox-esr you should adjust the icon in src/plasma-runner-firefoxprofilerunner.desktop
and change the path for the unzip command  
## Screenshots

#### Overview
The plugin gets triggered by firef.  
![Overview](https://raw.githubusercontent.com/alex1701c/Screenshots/master/FirefoxProfileRunner/overview.png)

#### Search
After the trigger word you can search the profiles.  
![Search](https://raw.githubusercontent.com/alex1701c/Screenshots/master/FirefoxProfileRunner/filter_profiles.png)

#### Private Window
You can add a -p flag to your query if you want firefox to open in a private window.  
![Private Window](https://raw.githubusercontent.com/alex1701c/Screenshots/master/FirefoxProfileRunner/private_window_flag.png)

#### Config Dialog  
In the config dialog you can change the name and order of your profiles.
Additionally you can sync the profiles from Firefox (in case you created/renamed them).  

If you uncheck the option to register the profiles globally you can not launch the profiles like in the last screenshot.  
If the following option is checked the option to launch a normal window with the default profile is not shown.
This is useful, because the default profile can be launched with the "Applications" plugin.  
The option to always show private windows is demonstrated in the next screenshot.
This is useful if you have only one/a few profiles.   
With the last checkbox selected the private window options will have the icon of the private windows in Firefox (as in the next screenshot).  

![Config Dialog](https://raw.githubusercontent.com/alex1701c/Screenshots/master/FirefoxProfileRunner/config_dialog.png)

#### Overview With Private Windows  
Here the options to always show private windows and to hide the default profile are enabled.  
(For demonstration purposes are here only 2 profiles).  
![Overview With Private Window](https://raw.githubusercontent.com/alex1701c/Screenshots/master/FirefoxProfileRunner/hide_default_show_private_windows.png)

#### Search from normal launcher
You can also search by name for the profiles in your launcher or with the "Applications" Krunner plugin.    
![Search from normal launcher](https://raw.githubusercontent.com/alex1701c/Screenshots/master/FirefoxProfileRunner/search_profiles_from_launcher.png)

