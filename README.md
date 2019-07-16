## Firefox Profile Runner

This plugin allows you to launch Firefox profiles from Krunner and your normal launcher.

The plugin uses a local copy of the firefox.desktop file. In this file it adds Desktop Action entries for
the profiles. Because of this you can search for the profiles in your normal launcher.  
If you make changes in your Firefox profile you can sync them manually in the config dialog or they will
be applied on startup. 

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
cp /usr/share/applications/firefox.desktop ~/.local/share/applications/firefox.desktop
git clone https://github.com/alex1701c/FirefoxProfileRunner
cd FirefoxProfileRunner
mkdir build
cd build
cmake -DQT_PLUGIN_INSTALL_DIR=`kf5-config --qt-plugins` ..
make
sudo make install
kquitapp5 krunner 2> /dev/null; kstart5 --windowclass krunner krunner > /dev/null 2>&1 &
```

After this you should see your runner in the system settings:

systemsettings5 (Head to "Search")


### Screenshots

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
![Config Dialog](https://raw.githubusercontent.com/alex1701c/Screenshots/master/FirefoxProfileRunner/config_dialog.png)

#### Search from normal launcher
You can also search for the profiles in your launcher or with the applications Krunner plugin.    
![Search from normal launcher](https://raw.githubusercontent.com/alex1701c/Screenshots/master/FirefoxProfileRunner/search_profiles_from_launcher.png)
