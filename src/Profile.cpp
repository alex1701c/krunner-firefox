//
// Created by alex on 14.07.19.
//

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
            defaultPath = config->group("Install4F96D1932A9F858E").readEntry("Default");
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
