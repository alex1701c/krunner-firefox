//
// Created by alex on 14.07.19.
//

#ifndef FIREFOXPROFILERUNNER_PROFILE_H
#define FIREFOXPROFILERUNNER_PROFILE_H


#include <QtCore/QString>
#include <QList>
#include <KSharedConfig>

class Profile {

public:
    QString name;
    QString launchName;
    QString path;
    QString firefoxDesktopFile;
    int priority;
    int privateWindowPriority;
    bool isDefault;
    bool isEdited;

    void writeSettings(KSharedConfigPtr firefoxConfig, const QString &installedProfile, int initialPriority = 0) const;

    void writeConfigChanges(KSharedConfigPtr firefoxConfig);

    void changeProfileRegistering(bool enable, KSharedConfigPtr firefoxConfig);

    QString getLaunchCommand() const;

    static QString getDefaultPath();

    static QList<Profile> getFirefoxProfiles();

    void syncDesktopFile(const QList<Profile> &profiles);

    QList<Profile> getCustomProfiles();

    static QString getDesktopFilePath();
};


#endif //FIREFOXPROFILERUNNER_PROFILE_H
