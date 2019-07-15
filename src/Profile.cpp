#include <KConfigCore/KConfigGroup>
#include <KConfigCore/KSharedConfig>
#include <QDir>
#include <QDebug>
#include "Profile.h"

QList<Profile> Profile::getFirefoxProfiles() {
    QList<Profile> profiles;
    KSharedConfigPtr config = KSharedConfig::openConfig(QDir::homePath() + "/" + ".mozilla/firefox/profiles.ini");
    QStringList configs = config->groupList().filter(QRegExp(R"(Install.*|Profile.*)"));
    QString defaultPath = getDefaultPath();

    for (const auto &profileEntry: configs.filter(QRegExp(R"(Profile.*)"))) {
        Profile profile;
        KConfigGroup profileConfig = config->group(profileEntry);
        profile.name = profileConfig.readEntry("Name");
        profile.path = profileConfig.readEntry("Path");
        profile.isDefault = profileConfig.readEntry("Path") == defaultPath;
        profiles.append(profile);
    }
    std::sort(profiles.begin(),profiles.end(), profileSmallerPriority);
    return profiles;
}

void Profile::syncDesktopFile(const QList<Profile> &profiles) {
    KSharedConfigPtr firefoxConfig = KSharedConfig::openConfig(
            QDir::homePath() + "/" + ".local/share/applications/firefox.desktop"
    );
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
                if (profile.name != firefoxConfig->group(installedProfile).readEntry("LaunchName")) {
#ifndef prod
                    qInfo() << "Update " << profile.name;
#endif
                    profile.writeSettings(firefoxConfig, installedProfile);
                }
            }
        }
        if (!found) {
#ifndef prod
            qInfo() << "Delete " << installedProfile;
#endif
            firefoxConfig->deleteGroup(installedProfile);
            deleted.append(installedProfile.remove("Desktop Action "));
        }
    }
    // Delete group and remove entry from Actions
    if (!deleted.isEmpty()) {
        for (const auto &del:deleted) {
            generalConfig.writeEntry("Actions", generalConfig.readEntry("Actions").remove(del + ";"));
        }
        generalConfig.sync();
    }
    // Add group and register action
    for (const auto &profile:profiles) {
        if (!firefoxConfig->hasGroup("Desktop Action new-window-with-profile-" + profile.path)) {
#ifndef prod
            qInfo() << "Install  " << profile.name;
#endif
            profile.writeSettings(firefoxConfig, "Desktop Action new-window-with-profile-" + profile.path);
            newInstalls.append("new-window-with-profile-" + profile.path + ";");
        }
    }
    generalConfig.writeEntry("Actions", generalConfig.readEntry("Actions") + newInstalls);


}

void Profile::writeSettings(KSharedConfigPtr firefoxConfig, const QString &installedProfile) const {
    KConfigGroup profileConfig = firefoxConfig->group(installedProfile);
    profileConfig.writeEntry("Name", this->name);
    profileConfig.writeEntry("LaunchName", this->name);
    profileConfig.writeEntry("Edited", false);
    profileConfig.writeEntry("Priority", 0);
    profileConfig.writeEntry("Exec", "firefox -P \"" + this->name + "\"");
}

QList<Profile> Profile::getCustomProfiles() {
    QList<Profile> profiles;
    KSharedConfigPtr firefoxConfig = KSharedConfig::openConfig(
            QDir::homePath() + "/" + ".local/share/applications/firefox.desktop"
    );
    QStringList installedProfiles = firefoxConfig->groupList().filter(QRegExp("Desktop Action new-window-with-profile-.*"));
    QString defaultPath = getDefaultPath();

    for (const auto &profileGroupName:installedProfiles) {
        auto profileGroup = firefoxConfig->group(profileGroupName);

        Profile profile;
        profile.name = profileGroup.readEntry("Name");
        profile.launchName = profileGroup.readEntry("LaunchName");
        profile.path = QString(profileGroupName).remove("Desktop Action new-window-with-profile-");
        profile.isDefault = profile.path == defaultPath;
        profile.isEdited = profileGroup.readEntry("Edited", "false") == "true";
        profile.priority = profileGroup.readEntry("Priority", "0").toInt();

        profiles.append(profile);
    }
    return profiles;
}

QString Profile::getDefaultPath() {
    KSharedConfigPtr config = KSharedConfig::openConfig(QDir::homePath() + "/" + ".mozilla/firefox/profiles.ini");
    QStringList configs = config->groupList().filter(QRegExp(R"(Install.*)"));

    return config->group(configs.first()).readEntry("Default", "");
}

bool Profile::profileSmallerPriority(const Profile &profile1, const Profile &profile2) {
    return profile1.priority < profile2.priority;
}

