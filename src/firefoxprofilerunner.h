#ifndef FIREFOXPROFILERUNNER_H
#define FIREFOXPROFILERUNNER_H

#include <KRunner/AbstractRunner>
#include "profile/ProfileManager.h"
#include <QtCore/QFileSystemWatcher>
#include <QRegularExpression>
#include "profile/Profile.h"

class FirefoxProfileRunner : public Plasma::AbstractRunner {
Q_OBJECT

public:
    FirefoxProfileRunner(QObject *parent, const QVariantList &args);

    QLatin1String prefix = QLatin1String("fire");
    QFileSystemWatcher watcher;
    QString launchCommand;
    QRegularExpression filterRegex = QRegularExpression(R"(^fire\w*(?: (.+))$)");
    const QRegExp privateWindowFlagRegex = QRegExp(" -p *$");
    QList<Profile> profiles;
    bool hideDefaultProfile, showAlwaysPrivateWindows, proxychainsIntegrated, proxychainsForceNewInstance;
    QIcon firefoxIcon;
    const QIcon firefoxPrivateWindowIcon = QIcon::fromTheme("private_browsing_firefox");

    QList<Plasma::QueryMatch> createProfileMatches(const QString &filter, bool privateWindow);

    Plasma::QueryMatch createMatch(const QString &text, const QMap<QString, QVariant> &data, float relevance);

public: // Plasma::AbstractRunner API
    void match(Plasma::RunnerContext &context) override;

    void run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match) override;

protected Q_SLOTS:

    void reloadPluginConfiguration(const QString &configFile = "");
};

#endif
