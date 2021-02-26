#ifndef FIREFOXPROFILERUNNER_H
#define FIREFOXPROFILERUNNER_H

#include <KRunner/AbstractRunner>
#include "profile/ProfileManager.h"
#include <QtCore/QFileSystemWatcher>
#include <QRegularExpression>
#include "profile/Profile.h"

class FirefoxRunner : public Plasma::AbstractRunner {
Q_OBJECT

public:
    FirefoxRunner(QObject *parent, const QVariantList &args);

    // NOTE: Prefixes need to be included in filterRegex.
    QLatin1String shortPrefix = QLatin1String("ff");
    QLatin1String mediumPrefix = QLatin1String("fire");
    QRegularExpression filterRegex = QRegularExpression(
            R"(^(?:ff|fire\w*)(?: (.+))$)",
            QRegularExpression::CaseInsensitiveOption
    );
    const QRegularExpression privateWindowFlagRegex = QRegularExpression(
            R"((\s+-p\b))",
            QRegularExpression::CaseInsensitiveOption
    );

    QFileSystemWatcher watcher;
    QString launchCommand;
    QList<Profile> profiles;
    bool hideDefaultProfile, showAlwaysPrivateWindows, proxychainsIntegrated, proxychainsForceNewInstance;
    QIcon firefoxIcon;
    const QIcon firefoxPrivateWindowIcon = QIcon::fromTheme("private_browsing_firefox", QIcon::fromTheme("view-private"));
    bool privateWindowsAsActions;
    QList<QAction *> matchActions;
    const QString proxychainsDisplayPrefix = "Proxychains: ";

    QList<Plasma::QueryMatch> createProfileMatches(const QString &filter, bool privateWindow);
    Plasma::QueryMatch createMatch(const QString &text, const QMap<QString, QVariant> &data, float relevance);

public: // Plasma::AbstractRunner API
    void init() override;
    void reloadPluginConfiguration(const QString &configFile = "");
    void match(Plasma::RunnerContext &context) override;
    QList<QAction *> actionsForMatch(const Plasma::QueryMatch &match) override;
    void run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match) override;
};

#endif
