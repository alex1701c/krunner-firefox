#ifndef FIREFOXPROFILERUNNER_PROFILEMANAGER_H
#define FIREFOXPROFILERUNNER_PROFILEMANAGER_H

#include <QtCore/QString>
#include <QList>
#include <KSharedConfig>
#include "Profile.h"

class ProfileManager {
public:

    QString firefoxDesktopFile, launchCommand, defaultPath, firefoxProfilesIniPath;

    ProfileManager();

    QList<Profile> syncAndGetCustomProfiles(bool forceSync = false);

    QList<Profile> getFirefoxProfiles();

    QList<Profile> getCustomProfiles(KSharedConfigPtr firefoxConfig);

    void syncDesktopFile(const QList<Profile> &profiles, KSharedConfigPtr firefoxConfig, const KConfigGroup &config);

    void changeProfileRegistering(bool enableNormal, bool enablePrivate, bool enableProxychainsExtra, KSharedConfigPtr firefoxConfig);

    QString getLaunchCommand() const;

    QString getDefaultProfilePath() const;

    QString getDesktopFilePath() const;
};


#endif //FIREFOXPROFILERUNNER_PROFILEMANAGER_H
