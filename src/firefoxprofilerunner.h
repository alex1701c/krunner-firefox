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
    void init() override;
    void reloadPluginConfiguration(const QString &configFile = QString());
    void match(Plasma::RunnerContext &context) override;
    QList<QAction *> actionsForMatch(const Plasma::QueryMatch &match) override;
    void run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match) override;

private:
    QList<Plasma::QueryMatch> createProfileMatches(const QString &filter, bool privateWindow);
    Plasma::QueryMatch createMatch(const QString &text, const QMap<QString, QVariant> &data, float relevance);

    QFileSystemWatcher watcher;
    QString launchCommand;
    const QRegularExpression filterRegex;
    QList<Profile> profiles;
    bool hideDefaultProfile, showAlwaysPrivateWindows, proxychainsIntegrated, proxychainsForceNewInstance;
    QIcon firefoxIcon;
    QIcon firefoxPrivateWindowIcon;
    bool privateWindowsAsActions;
    const QString proxychainsDisplayPrefix;

};

#endif
