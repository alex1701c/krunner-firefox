#ifndef FIREFOXPROFILERUNNER_PROFILEMANAGER_H
#define FIREFOXPROFILERUNNER_PROFILEMANAGER_H

#include "Profile.h"
#include <Config.h>
#include <KSharedConfig>
#include <QList>
#include <QString>

class ProfileManager
{
public:
    QString firefoxDesktopFile, launchCommand, defaultPath, firefoxProfilesIniPath;

    ProfileManager();
    void initializeConfigFiles();

    QList<Profile> syncAndGetCustomProfiles(bool forceSync = false);
    QList<Profile> getFirefoxProfiles();
    QList<Profile> getCustomProfiles(KSharedConfigPtr firefoxConfig);

    void syncDesktopFile(const QList<Profile> &profiles, KSharedConfigPtr firefoxConfig, const KConfigGroup &config);
    void changeProfileRegistering(bool enableNormal, bool enablePrivate, bool enableProxychainsExtra, KSharedConfigPtr firefoxConfig);

    QString getLaunchCommand() const;
    QString getDefaultProfilePath() const;
    static QString getDesktopFilePath(bool quiet = false);
};

#endif // FIREFOXPROFILERUNNER_PROFILEMANAGER_H
