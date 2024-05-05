#pragma once

#include <KSharedConfig>
#include <QList>
#include <QString>

class Profile
{
public:
    QString name;
    QString launchCommand;
    QString launchName;
    QString path;
    int priority;
    bool isDefault;
    bool isEdited;
    int privateWindowPriority = 0;

    void writeSettings(KSharedConfigPtr firefoxConfig, int initialPriority = 0) const;

    void writeConfigChanges(KSharedConfigPtr firefoxConfig) const;
};
QDebug operator<<(QDebug debug, const Profile &profile);
