#ifndef FIREFOXPROFILERUNNER_H
#define FIREFOXPROFILERUNNER_H

#include <KRunner/AbstractRunner>
#include "profile/ProfileManager.h"
#include <QtCore/QFileSystemWatcher>
#include <QRegularExpression>
#include "profile/Profile.h"

#define QSL(text) QStringLiteral(text)

class FirefoxRunner : public Plasma::AbstractRunner {
Q_OBJECT

public:
    FirefoxRunner(QObject *parent, const QVariantList &args);

    const QLatin1String prefix = QLatin1String("fire");
    QFileSystemWatcher watcher;
    QString launchCommand;
    QRegularExpression filterRegex = QRegularExpression(QSL(R"(^fire\w*(?: (.+))$)"));
    const QRegularExpression privateWindowFlagRegex = QRegularExpression(QSL(" -p *$"));
    QList<Profile> profiles;
    bool hideDefaultProfile, showAlwaysPrivateWindows, proxychainsIntegrated, proxychainsForceNewInstance;
    QIcon firefoxIcon;
    const QIcon firefoxPrivateWindowIcon = QIcon::fromTheme(QSL("private_browsing_firefox"), QIcon::fromTheme(QSL("view-private")));
    bool privateWindowsAsActions;
    QList<QAction *> matchActions;
    const QString proxychainsDisplayPrefix = QSL("Proxychains: ");

    QList<Plasma::QueryMatch> createProfileMatches(const QString &filter, bool privateWindow);
    Plasma::QueryMatch createMatch(const QString &text, const QMap<QString, QVariant> &data, float relevance);

public: // Plasma::AbstractRunner API
    void init() override;
    void reloadPluginConfiguration(const QString &configFile = QString());
    void match(Plasma::RunnerContext &context) override;
    QList<QAction *> actionsForMatch(const Plasma::QueryMatch &match) override;
    void run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match) override;
};

#endif
