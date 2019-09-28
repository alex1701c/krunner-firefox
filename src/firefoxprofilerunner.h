#ifndef FIREFOXPROFILERUNNER_H
#define FIREFOXPROFILERUNNER_H

#include <KRunner/AbstractRunner>
#include <profile/ProfileManager.h>
#include "profile/Profile.h"

class FirefoxProfileRunner : public Plasma::AbstractRunner {
Q_OBJECT

public:
    FirefoxProfileRunner(QObject *parent, const QVariantList &args);

    QString launchCommand;
    QList<Profile> profiles;
    KConfigGroup config;
    ProfileManager profileManager;
    bool hideDefaultProfile, showAlwaysPrivateWindows, proxychainsIntegrated, proxychainsForceNewInstance;
    QString proxychainsIntegration;
    QIcon firefoxIcon, firefoxPrivateWindowIcon;

    QList<Plasma::QueryMatch> createProfileMatches(const QString &filter, bool privateWindow);

    Plasma::QueryMatch createMatch(const QString &text, const QMap<QString, QVariant> &data, float relevance);

public: // Plasma::AbstractRunner API
    void match(Plasma::RunnerContext &context) override;

    void run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match) override;

    void reloadConfiguration() override;
};

#endif
