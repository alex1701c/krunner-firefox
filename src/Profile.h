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
    bool isDefault;
    bool isEdited;
    int priority;

    static QString getDefaultPath();

    static bool profileSmallerPriority(const Profile &profile1, const Profile &profile2);

    void writeSettings(KSharedConfigPtr firefoxConfig, const QString &installedProfile) const;

    static QList<Profile> getFirefoxProfiles();

    static void syncDesktopFile(const QList<Profile> &profiles);

    static QList<Profile> getCustomProfiles();

};


#endif //FIREFOXPROFILERUNNER_PROFILE_H
