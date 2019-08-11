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
    int priority;
    int privateWindowPriority;
    bool isDefault;
    bool isEdited;

    void writeSettings(KSharedConfigPtr firefoxConfig, const QString &profile, const QString &command,
                       int initialPriority = 0) const;

    void writeConfigChanges(KSharedConfigPtr firefoxConfig);
};


#endif //FIREFOXPROFILERUNNER_PROFILE_H
