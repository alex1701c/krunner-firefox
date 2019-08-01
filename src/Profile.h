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

    void writeSettings(KSharedConfigPtr firefoxConfig, const QString &installedProfile) const;

    void writeConfigChanges(KSharedConfigPtr firefoxConfig);

    int priority;

    static void changeProfileRegistering(bool enable,  KSharedConfigPtr firefoxConfig);

    static QString getDefaultPath();

    static QList<Profile> getFirefoxProfiles();

    static void syncDesktopFile(const QList<Profile> &profiles);

    static QList<Profile> getCustomProfiles();

};


#endif //FIREFOXPROFILERUNNER_PROFILE_H
