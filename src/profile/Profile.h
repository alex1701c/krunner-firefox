#ifndef FIREFOXPROFILERUNNER_PROFILE_H
#define FIREFOXPROFILERUNNER_PROFILE_H

#include <QtCore/QString>
#include <QList>
#include <KSharedConfig>

class Profile {
public:
    QString name;
    QString launchCommand;
    QString launchName;
    QString path;
    int priority;
    bool isDefault;
    bool isEdited;
    bool launchNormalWindowWithProxychains;
    bool extraNormalWindowProxychainsLaunchOption = false;
    int extraNormalWindowProxychainsOptionPriority = 0;

    int privateWindowPriority = 0;
    bool launchPrivateWindowWithProxychains;
    bool extraPrivateWindowProxychainsLaunchOption = false;
    int extraPrivateWindowProxychainsOptionPriority = 0;


    void writeSettings(KSharedConfigPtr firefoxConfig, int initialPriority = 0) const;

    void writeConfigChanges(KSharedConfigPtr firefoxConfig, const QString &forceNewInstance = "");
};


#endif //FIREFOXPROFILERUNNER_PROFILE_H
