# Firefox Profile Runner
This plugin allows you to launch Firefox profiles from Krunner and your normal launcher.
It saves the Desktop Actions for the profiles in a local copy of the firefox.desktop file.
This means you can search for the profiles in your normal launcher.

- Saves the Desktop Actions for the profiles in a local copy of the `firefox.desktop` file. This means you can search for the profiles in your normal launcher.
- Compatible with Firefox-ESR.

# Installation

## 1 Dependencies


<details>
<summary><b>Debian/Ubuntu</b></summary>

Plasma5:  
```bash install-ubuntu-plasma5
sudo apt install git cmake extra-cmake-modules build-essential libkf5runner-dev libkf5i18n-dev libkf5service-dev libkf5kcmutils-dev libkf5dbusaddons-bin
```
Plasma6:  
```bash install-ubuntu-plasma6
sudo apt install git cmake extra-cmake-modules build-essential libkf6runner-dev libkf6i18n-dev libkf6service-dev libkf6kcmutils-dev libkf6dbusaddons-bin
```

</details>

<details>
<summary><b>OpenSUSE</b></summary>

Plasma5:  
```bash install-opensuse-plasma5
sudo zypper install git cmake extra-cmake-modules ki18n-devel krunner-devel kcmutils-devel kservice-devel kdbusaddons-tools libQt5Test-devel
```
Plasma6:  
```bash install-opensuse-plasma6
sudo zypper install git cmake kf6-extra-cmake-modules kf6-ki18n-devel kf6-krunner-devel kf6-kcmutils-devel kf6-kservice-devel kf6-kdbusaddons-tools qt6-test-devel
```

</details>

<details>
<summary><b>Fedora</b></summary>

Plasma5:  
```bash install-fedora-plasma5
sudo dnf install git cmake extra-cmake-modules kf5-ki18n-devel kf5-krunner-devel kf5-kcmutils-devel kf5-kservice-devel
```
Plasma6:  
```bash install-fedora-plasma6
sudo dnf install git cmake extra-cmake-modules kf6-ki18n-devel kf6-krunner-devel kf6-kcmutils-devel kf6-kservice-devel 
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

# Screenshots

#### Overview
The plugin gets triggered by `firef`.

![Overview](https://raw.githubusercontent.com/alex1701c/Screenshots/master/FirefoxProfileRunner/runner_profiles_overview.png)

#### Search
After the trigger word, you can search the profiles. 

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
