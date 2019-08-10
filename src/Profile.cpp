#include <KConfigCore/KConfigGroup>
#include <KConfigCore/KSharedConfig>
#include <QDir>
#include <QDebug>
#include "Profile.h"

QList<Profile> Profile::getFirefoxProfiles() {
    QList<Profile> profiles;
    KSharedConfigPtr config = KSharedConfig::openConfig(QDir::homePath() + "/" + ".mozilla/firefox/profiles.ini");
    QStringList configs = config->groupList().filter(QRegExp(R"(Install.*|Profile.*)"));

    for (const auto &profileEntry: configs.filter(QRegExp(R"(Profile.*)"))) {
        Profile profile;
        KConfigGroup profileConfig = config->group(profileEntry);
        profile.launchName = profileConfig.readEntry("Name");
        profile.path = profileConfig.readEntry("Path");
        profiles.append(profile);
    }
    return profiles;
}

void Profile::syncDesktopFile(const QList<Profile> &profiles) {
    firefoxDesktopFile = getDesktopFilePath();
    if (firefoxDesktopFile == "<error>") return;
    KSharedConfigPtr firefoxConfig = KSharedConfig::openConfig(firefoxDesktopFile);
    KConfigGroup generalConfig = firefoxConfig->group("Desktop Entry");
    QStringList installedProfiles = firefoxConfig->groupList().filter(QRegExp("Desktop Action new-window-with-profile-.*"));

    QStringList deleted;
    QString newInstalls;

    // Update/mark to delete installed profiles
    for (auto &installedProfile:installedProfiles) {
        bool found = false;
        for (const auto &profile:profiles) {
            if (installedProfile == "Desktop Action new-window-with-profile-" + profile.path) {
                found = true;
                if (profile.launchName != firefoxConfig->group(installedProfile).readEntry("LaunchName")) {
#ifdef status_dev
                    qInfo() << "Update " << profile.launchName;
#endif
                    profile.writeSettings(firefoxConfig, installedProfile);
                }
            }
        }
        if (!found) {
#ifdef status_dev
            qInfo() << "Delete " << installedProfile;
#endif
            firefoxConfig->deleteGroup(installedProfile);
        }
    }
    // Add group and register action
    int idx = 1;
    for (const auto &profile:profiles) {
        if (!firefoxConfig->hasGroup("Desktop Action new-window-with-profile-" + profile.path)) {
#ifdef status_dev
            qInfo() << "Install  " << profile.launchName;
#endif
            profile.writeSettings(firefoxConfig, "Desktop Action new-window-with-profile-" + profile.path, idx);
            ++idx;
        }
    }
    changeProfileRegistering(firefoxConfig->group("Settings").readEntry("registerProfiles", "true") == "true", firefoxConfig);
}

void Profile::writeSettings(KSharedConfigPtr firefoxConfig, const QString &installedProfile, int initialPriority) const {
    KConfigGroup profileConfig = firefoxConfig->group(installedProfile);
    if (profileConfig.readEntry("Edited", "false") != "true") {
        profileConfig.writeEntry("Name", this->name.isEmpty() ? this->launchName : this->name);
    }
    profileConfig.writeEntry("LaunchName", this->launchName);
    profileConfig.writeEntry("Edited", false);
    profileConfig.writeEntry("Priority", profileConfig.readEntry("Priority", QString::number(initialPriority)));
    getDesktopFilePath();
    profileConfig.writeEntry("Exec", getLaunchCommand() + " -P \"" + this->launchName + "\"");
}

QList<Profile> Profile::getCustomProfiles() {
    QList<Profile> profiles;
    firefoxDesktopFile = getDesktopFilePath();
    if (firefoxDesktopFile == "<error>") return profiles;
    KSharedConfigPtr firefoxConfig = KSharedConfig::openConfig(firefoxDesktopFile);
    QStringList installedProfiles = firefoxConfig->groupList().filter(QRegExp("Desktop Action new-window-with-profile-.*"));
    QString defaultPath = getDefaultPath();

    for (const auto &profileGroupName:installedProfiles) {
        auto profileGroup = firefoxConfig->group(profileGroupName);

        Profile profile;
        profile.name = profileGroup.readEntry("Name");
        profile.launchName = profileGroup.readEntry("LaunchName");
        profile.path = QString(profileGroupName).remove("Desktop Action new-window-with-profile-");
        if (defaultPath == "<invalid>") defaultPath = profile.path;
        profile.isDefault = profile.path == defaultPath;
        profile.isEdited = profileGroup.readEntry("Edited", "false") == "true";
        profile.priority = profileGroup.readEntry("Priority", "0").toInt();
        profile.privateWindowPriority = profileGroup.readEntry("PrivateWindowPriority", "0").toInt();

        profiles.append(profile);
    }
    std::sort(profiles.begin(), profiles.end(), [](const Profile &profile1, const Profile &profile2) -> bool {
        return profile1.priority > profile2.priority;
    });
    return profiles;
}

QString Profile::getDefaultPath() {
    KSharedConfigPtr config = KSharedConfig::openConfig(QDir::homePath() + "/" + ".mozilla/firefox/profiles.ini");
    QStringList configs = config->groupList();
    QStringList installConfig = configs.filter(QRegExp(R"(Install.*)"));
    QString defaultPath;
    if (installConfig.size() != 0) {
        defaultPath = config->group(installConfig.first()).readEntry("Default", "");
    }
    if (!defaultPath.isEmpty()) {
        return defaultPath;
    }
    for (const auto &profileName:config->groupList().filter(QRegExp(R"(Profile.*)"))) {
        const auto profile = config->group(profileName);
        if (profile.readEntry("Default", "0") == "1") return profile.readEntry("Path");
    }
    return "<invalid>";
}


void Profile::writeConfigChanges(KSharedConfigPtr firefoxConfig) {
    KConfigGroup profileConfig = firefoxConfig->group("Desktop Action new-window-with-profile-" + this->path);
    profileConfig.writeEntry("Name", this->name);
    profileConfig.writeEntry("Edited", true);
    profileConfig.writeEntry("Priority", this->priority);
    profileConfig.writeEntry("PrivateWindowPriority", this->privateWindowPriority);
}

void Profile::changeProfileRegistering(bool enable, KSharedConfigPtr firefoxConfig) {
    QString registeredActions = "new-window;new-private-window;";
    if (firefoxDesktopFile.endsWith("firefox-esr.desktop")) registeredActions.clear();
    if (enable) {
        for (const auto &groupName:firefoxConfig->groupList()) {
            if (groupName.startsWith("Desktop Action new-window-with-profile")) {
                registeredActions.append(QString(groupName).remove("Desktop Action ") + ";");
            }
        }
    }
    firefoxConfig->group("Desktop Entry").writeEntry("Actions", registeredActions);
}

QString Profile::getDesktopFilePath() {
    QString firefoxDesktopFile(QDir::homePath() + "/" + ".local/share/applications/firefox.desktop");
    if (!QFile::exists(firefoxDesktopFile)) {
        firefoxDesktopFile = QDir::homePath() + "/" + ".local/share/applications/firefox-esr.desktop";
        if (!QFile::exists(firefoxDesktopFile)) {
            qWarning() << "Can not find a firefox.desktop or firefox-esr.desktop file in ~/.local/share/applications/";
            return "<error>";
        }
    }
    return firefoxDesktopFile;
}

QString Profile::getLaunchCommand() const {
    return KSharedConfig::openConfig(getDesktopFilePath())->group("Desktop Entry").readEntry("Exec").remove(" %u");
}
