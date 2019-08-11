#ifndef FIREFOXPROFILERUNNER_PROFILEMANAGER_H
#define FIREFOXPROFILERUNNER_PROFILEMANAGER_H

#include <QtCore/QString>
#include <QList>
#include <KSharedConfig>
#include "Profile.h"

class ProfileManager {
public:

    QString firefoxDesktopFile, launchCommand, defaultPath;
    KSharedConfigPtr firefoxProfilesIni, firefoxConfig;

    ProfileManager();

    QList<Profile> getFirefoxProfiles();

    QList<Profile> getCustomProfiles();

    void syncDesktopFile(const QList<Profile> &profiles);

    void changeProfileRegistering(bool enable);

    QString getLaunchCommand() const;

    QString getDefaultProfilePath() const;

    QString getDesktopFilePath() const;
};


#endif //FIREFOXPROFILERUNNER_PROFILEMANAGER_H
