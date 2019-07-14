//
// Created by alex on 14.07.19.
//

#ifndef FIREFOXPROFILERUNNER_PROFILE_H
#define FIREFOXPROFILERUNNER_PROFILE_H


#include <QtCore/QString>
#include <QList>

class Profile {

public:
    QString name;
    QString path;
    bool isDefault;

    static QList<Profile> getProfiles();

    static void syncDesktopFile(const QList<Profile> &profiles);

};


#endif //FIREFOXPROFILERUNNER_PROFILE_H
