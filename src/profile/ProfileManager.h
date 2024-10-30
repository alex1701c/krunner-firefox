#pragma once

#include "Profile.h"
#include <Config.h>
#include <KSharedConfig>
#include <QList>
#include <QString>

class ProfileManager
{
public:
    ProfileManager();

    QList<Profile> syncAndGetCustomProfiles(KConfigGroup &grp, bool forceSync = false);
    QList<Profile> getFirefoxProfiles();
    QList<Profile> getCustomProfiles(KSharedConfigPtr firefoxConfig);

    void syncDesktopFile(const QList<Profile> &profiles, KSharedConfigPtr firefoxConfig, const KConfigGroup &config);
    void changeProfileRegistering(bool enableNormal, bool enablePrivate, KSharedConfigPtr firefoxConfig);

    QString getLaunchCommand() const;
    QString getDefaultProfilePath() const;
    QString getDesktopFilePath(bool quiet = false);

    QString iconForExecutable() const;

    QString firefoxDesktopFile, launchCommand, defaultPath, firefoxProfilesIniPath;

private:
    void initializeDesktopFileCopy();
    QStringList firefoxDesktopNames{
        QStringLiteral("firefox"),
        QStringLiteral("org.mozilla.firefox"),
        QStringLiteral("firefox-esr"),
    };
};
