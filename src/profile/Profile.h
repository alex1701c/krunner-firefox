#ifndef FIREFOXPROFILERUNNER_PROFILE_H
#define FIREFOXPROFILERUNNER_PROFILE_H

#include <QtCore/QString>
#include <QList>
#include <KSharedConfig>

class Profile {

public:
    QString name;
    // TODO Fill value
    QString launchCommand;
    QString launchName;
    QString path;
    int priority;
    bool isDefault;
    bool isEdited;
    bool launchNormalWindowWithProxychains;
    bool extraNormalWindowProxychainsLaunchOption;
    int extraNormalWindowProxychainsOptionPriority;

    int privateWindowPriority;
    bool launchPrivateWindowWithProxychains;
    bool extraprivateWindowProxychainsLaunchOption;
    int extraPrivateWindowProxychainsOptionPriority;


    void writeSettings(KSharedConfigPtr firefoxConfig, const QString &profile, const QString &command,
                       int initialPriority = 0) const;

    void writeConfigChanges(KSharedConfigPtr firefoxConfig, const QString &forceNewInstance = "");
};


#endif //FIREFOXPROFILERUNNER_PROFILE_H
