#include <KConfigCore/KConfigGroup>
#include <KConfigCore/KSharedConfig>
#include <QDir>
#include <QDebug>
#include "Profile.h"

QList<Profile> Profile::getProfiles() {
    QList<Profile> profiles;
    KSharedConfigPtr config = KSharedConfig::openConfig(QDir::homePath() + "/" + ".mozilla/firefox/profiles.ini");
    QStringList configs = config->groupList().filter(QRegExp(R"(Install.*|Profile.*)"));
    QString defaultPath = "";

    for (const auto &c:configs) {
        if (c.startsWith("Install")) {
            defaultPath = config->group(c).readEntry("Default");
        }
    }
    for (const auto &profileEntry: configs.filter(QRegExp(R"(Profile.*)"))) {
        Profile profile;
        KConfigGroup profileConfig = config->group(profileEntry);
        profile.name = profileConfig.readEntry("Name");
        profile.path = profileConfig.readEntry("Path");
        profile.isDefault = profileConfig.readEntry("Path") == defaultPath;
        profiles.append(profile);
    }

    return profiles;
}

void Profile::syncDesktopFile(const QList<Profile> &profiles) {
    KSharedConfigPtr firefoxConfig = KSharedConfig::openConfig(
            QDir::homePath() + "/" + ".local/share/applications/firefox.desktop"
    );
    KConfigGroup generalConfig = firefoxConfig->group("Desktop Entry");
    QStringList installedProfiles = firefoxConfig->groupList().filter(
            QRegExp("Desktop Action new-window-with-profile-.*"));

    QStringList deleted;
    QString newInstalls;

    // Update/mark to delete installed profiles
    for (auto &installedProfile:installedProfiles) {
        for (const auto &profile:profiles) {
            if (installedProfile == "Desktop Action new-window-with-profile-" + profile.name) {
                profile.writeSettings(firefoxConfig, installedProfile);
                continue;
            }
        }
        firefoxConfig->deleteGroup(installedProfile);
        deleted.append(installedProfile.remove("Desktop Action "));
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
        if (!firefoxConfig->hasGroup("Desktop Action new-window-with-profile-" + profile.name)) {
            profile.writeSettings(firefoxConfig, "Desktop Action new-window-with-profile-" + profile.name);
            newInstalls.append("new-window-with-profile-" + profile.name + ";");
        }
    }
    generalConfig.writeEntry("Actions", generalConfig.readEntry("Actions") + newInstalls);


}

void Profile::writeSettings(KSharedConfigPtr firefoxConfig, const QString &installedProfile) const {
    KConfigGroup profileConfig = firefoxConfig->group(installedProfile);
    if (this->isDefault) {
        profileConfig.writeEntry("Name", this->name + " (default)");
    } else {
        profileConfig.writeEntry("Name", this->name);
    }
    profileConfig.writeEntry("Exec", "firefox -P " + this->name);
}
