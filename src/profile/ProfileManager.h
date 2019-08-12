#ifndef FIREFOXPROFILERUNNER_PROFILEMANAGER_H
#define FIREFOXPROFILERUNNER_PROFILEMANAGER_H

#include <QtCore/QString>
#include <QList>
#include <KSharedConfig>
#include "Profile.h"

class ProfileManager {
public:

    QString firefoxDesktopFile, launchCommand, defaultPath;

    ProfileManager();

    QList<Profile> syncAndGetCustomProfiles(bool sync = true);

    QList<Profile> getFirefoxProfiles();

    QList<Profile> getCustomProfiles(KSharedConfigPtr firefoxConfig);

    void syncDesktopFile(const QList<Profile> &profiles, KSharedConfigPtr firefoxConfig);

    void changeProfileRegistering(bool enable, KSharedConfigPtr firefoxConfig);

    QString getLaunchCommand() const;

    QString getDefaultProfilePath() const;

    QString getDesktopFilePath() const;
};


#endif //FIREFOXPROFILERUNNER_PROFILEMANAGER_H
