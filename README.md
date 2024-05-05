# Firefox Profile Runner
This plugin allows you to launch Firefox profiles from Krunner and your normal launcher.
It saves the Desktop Actions for the profiles in a local copy of the firefox.desktop file.
This mean you can search for the profiles in your normal launcher.

- Saves the Desktop Actions for the profiles in a local copy of the `firefox.desktop` file. This mean you can search for the profiles in your normal launcher.
- Compatible with Firefox-ESR.

# Installation

## 1 Dependencies

<details>
<summary><b>Debian/Ubuntu:</b></summary>
  
```shell
sudo apt install cmake extra-cmake-modules build-essential libkf5runner-dev libkf5textwidgets-dev qtdeclarative5-dev gettext libkf5kcmutils-dev
```
  
</details>

<details>
<summary><b>OpenSUSE:</b></summary>
  
```shell
sudo zypper install cmake extra-cmake-modules libQt5Widgets5 libQt5Core5 libqt5-qtlocation-devel ki18n-devel ktextwidgets-devel kservice-devel krunner-devel gettext-tools kconfigwidgets-devel kcmutils-devel
```

</details>

<details>
<summary><b>Fedora:</b></summary>
  
```shell
sudo dnf install cmake extra-cmake-modules kf5-ki18n-devel kf5-kservice-devel kf5-krunner-devel kf5-ktextwidgets-devel gettext kf5-kcmutils-devel
```
  
</details>

<details>
<summary><b>Arch (Manjaro):</b></summary>
  
```shell
sudo pacman -S install cmake extra-cmake-modules kcmutils
```
  
</details>

## 2 Plugin

### A: Oneliner install

```shell
curl https://raw.githubusercontent.com/alex1701c/krunner-firefox/master/install.sh | bash
```

### B: Oneliner install (Without admin privileges)

```shell
curl https://raw.githubusercontent.com/alex1701c/krunner-firefox/master/install-user.sh | bash
```

### C: Manual install

```shell
git clone https://github.com/alex1701c/krunner-firefox
cd krunner-firefox
sudo bash install.sh
kquitapp5 krunner 2> /dev/null; kstart5 --windowclass krunner krunner > /dev/null 2>&1 &
```

To get the icon for the private window you have to extract it:
```shell
mkdir -p /tmp/firefoxprofile_installer
unzip /usr/lib/firefox/browser/omni.ja -c "chrome/browser/skin/classic/browser/privatebrowsing/favicon.svg" -d /tmp/firefoxprofile_installer > /dev/null 2>&1
sudo mv /tmp/firefoxprofile_installer/chrome/browser/skin/classic/browser/privatebrowsing/favicon.svg /usr/share/pixmaps/private_browsing_firefox.svg
rm -rf /tmp/firefoxprofile_installer
```
If you install it manually for Firefox-ESR you should adjust the icon in `src/plasma-runner-firefoxprofilerunner.desktop` and change the path for the unzip command

# Screenshots

#### Overview
The plugin gets triggered by `firef`.

![Overview](https://raw.githubusercontent.com/alex1701c/Screenshots/master/FirefoxProfileRunner/runner_profiles_overview.png)

#### Search
After the trigger word you can search the profiles. 

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

#### Overview With All Global Options
After the trigger word, you can search the profiles. 

![Overview With All Global Options](https://raw.githubusercontent.com/alex1701c/Screenshots/master/FirefoxProfileRunner/global_overview_proxychains.png)
