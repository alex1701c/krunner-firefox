#include <QtCore/QDir>
#include <QDebug>
#include <KConfigCore/KConfigGroup>
#include "ProfileManager.h"
#include "helper.h"

ProfileManager::ProfileManager() {
    firefoxDesktopFile = getDesktopFilePath();
    launchCommand = getLaunchCommand();
    defaultPath = getDefaultProfilePath();
}

QList<Profile> ProfileManager::syncAndGetCustomProfiles(bool forceSync) {
    KSharedConfigPtr firefoxConfig = KSharedConfig::openConfig(firefoxDesktopFile);
    const auto config = KSharedConfig::openConfig("krunnerrc")->group("Runners").group("FirefoxProfileRunner");
    if (forceSync || stringToBool(config.readEntry("automaticallyRegisterProfiles", "true"))) {
        QList<Profile> firefoxProfiles = getFirefoxProfiles();
        syncDesktopFile(firefoxProfiles, firefoxConfig);
#ifdef  status_dev
        qInfo() << "Synced profiles";
#endif
    }
    return getCustomProfiles(firefoxConfig);
}

QList<Profile> ProfileManager::getFirefoxProfiles() {
    QList<Profile> profiles;
    KSharedConfigPtr firefoxProfilesIni = KSharedConfig::openConfig(QDir::homePath() + "/" + ".mozilla/firefox/profiles.ini");
    QStringList configs = firefoxProfilesIni->groupList().filter(QRegExp(R"(Install.*|Profile.*)"));

    for (const auto &profileEntry: configs.filter(QRegExp(R"(Profile.*)"))) {
        Profile profile;
        KConfigGroup profileConfig = firefoxProfilesIni->group(profileEntry);
        profile.launchName = profileConfig.readEntry("Name");
        profile.path = profileConfig.readEntry("Path");
        profiles.append(profile);
    }
    return profiles;
}

QList<Profile> ProfileManager::getCustomProfiles(KSharedConfigPtr firefoxConfig) {
    QList<Profile> profiles;
    if (firefoxDesktopFile == "<error>") return profiles;
    firefoxConfig->sync();
    QStringList installedProfiles = firefoxConfig->groupList().filter(QRegExp("Desktop Action new-window-with-profile-.*"));
    for (const auto &profileGroupName:installedProfiles) {
        auto profileGroup = firefoxConfig->group(profileGroupName);
        if (!profileGroup.exists() || profileGroup.keyList().isEmpty()) continue;
        Profile profile;
        profile.name = profileGroup.readEntry("Name");
        profile.launchName = profileGroup.readEntry("LaunchName");
        profile.path = QString(profileGroupName).remove("Desktop Action new-window-with-profile-");
        if (defaultPath == "<invalid>") defaultPath = profile.path;
        profile.isDefault = profile.path == defaultPath;
        profile.isEdited = stringToBool(profileGroup.readEntry("Edited", "false"));
        profile.priority = profileGroup.readEntry("Priority", "0").toInt();
        profile.privateWindowPriority = profileGroup.readEntry("PrivateWindowPriority", "0").toInt();

        profiles.append(profile);
    }
    std::sort(profiles.begin(), profiles.end(), [](const Profile &profile1, const Profile &profile2) -> bool {
        return profile1.priority > profile2.priority;
    });
    return profiles;
}

void ProfileManager::syncDesktopFile(const QList<Profile> &profiles, KSharedConfigPtr firefoxConfig) {
    if (firefoxDesktopFile == "<error>") return;
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
                    profile.writeSettings(firefoxConfig, profile.path, launchCommand);
                }
            }
        }
        if (!found) {
#ifdef status_dev
            qInfo() << "Delete " << installedProfile;
#endif
            // Delete normal and private window Desktop Action
            firefoxConfig->deleteGroup(installedProfile);
            firefoxConfig->deleteGroup("Desktop Action new-private-window-with-profile-" +
                                       QString(installedProfile).remove("Desktop Action new-window-with-profile-"));
        }
    }
    // Add group and register action
    int idx = 1;
    for (const auto &profile:profiles) {
        if (!firefoxConfig->hasGroup("Desktop Action new-window-with-profile-" + profile.path)) {
#ifdef status_dev
            qInfo() << "Install  " << profile.launchName;
#endif
            // Write settings for normal and private window
            profile.writeSettings(firefoxConfig, profile.path, launchCommand, idx);
            ++idx;
        }
    }
    const auto config = KSharedConfig::openConfig("krunnerrc")->group("Runners").group("FirefoxProfileRunner");
    bool enableNormal = stringToBool(config.readEntry("registerNormalWindows", "true"));
    bool enablePrivate = stringToBool(config.readEntry("registerPrivateWindows", "true"));
    changeProfileRegistering(enableNormal, enablePrivate, firefoxConfig);
}

void ProfileManager::changeProfileRegistering(bool enableNormal, bool enablePrivate, KSharedConfigPtr firefoxConfig) {
    QString registeredActions = "new-window;new-private-window;";
    if (firefoxDesktopFile.endsWith("firefox-esr.desktop")) registeredActions.clear();

    for (const auto &groupName:firefoxConfig->groupList()) {
        if (enableNormal && groupName.startsWith("Desktop Action new-window-with-profile")) {
            registeredActions.append(QString(groupName).remove("Desktop Action ") + ";");
        }
        if (enablePrivate && groupName.startsWith("Desktop Action new-private-window-with-profile-")) {
            registeredActions.append(QString(groupName).remove("Desktop Action ") + ";");
        }
    }
    firefoxConfig->group("Desktop Entry").writeEntry("Actions", registeredActions);
}

QString ProfileManager::getLaunchCommand() const {
    return KSharedConfig::openConfig(firefoxDesktopFile)->group("Desktop Entry").readEntry("Exec").remove(" %u");
}

QString ProfileManager::getDefaultProfilePath() const {
    KSharedConfigPtr firefoxProfilesIni = KSharedConfig::openConfig(QDir::homePath() + "/" + ".mozilla/firefox/profiles.ini");
    QStringList configs = firefoxProfilesIni->groupList();
    QStringList installConfig = configs.filter(QRegExp(R"(Install.*)"));
    QString path;
    if (!installConfig.empty()) {
        path = firefoxProfilesIni->group(installConfig.first()).readEntry("Default", "");
    }
    if (!path.isEmpty()) {
        return path;
    }
    for (const auto &profileName:firefoxProfilesIni->groupList().filter(QRegExp(R"(Profile.*)"))) {
        const auto profile = firefoxProfilesIni->group(profileName);
        if (profile.readEntry("Default", "0") == "1") return profile.readEntry("Path");
    }
    return "<invalid>";
}

QString ProfileManager::getDesktopFilePath() const {
    QString file(QDir::homePath() + "/" + ".local/share/applications/firefox.desktop");
    if (!QFile::exists(file)) {
        file = QDir::homePath() + "/" + ".local/share/applications/firefox-esr.desktop";
        if (!QFile::exists(file)) {
            qWarning() << "Can not find a firefox.desktop or firefox-esr.desktop file in ~/.local/share/applications/";
            return "<error>";
        }
    }
    return file;
}
