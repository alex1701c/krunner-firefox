#pragma once

#include "profile/Profile.h"
#include <KRunner/AbstractRunner>
#include <QFileSystemWatcher>
#include <QRegularExpression>
#include <krunner_version.h>

#if KRUNNER_VERSION_MAJOR == 5
using namespace Plasma;
#include <QAction>
#else
using namespace KRunner;
#include <KRunner/Action>
#endif

class FirefoxRunner : public AbstractRunner
{
    Q_OBJECT

public:
    FirefoxRunner(QObject *parent, const KPluginMetaData &data, const QVariantList &args);

    // NOTE: Prefixes need to be included in filterRegex.
    QLatin1String shortPrefix = QLatin1String("ff");
    QLatin1String mediumPrefix = QLatin1String("fire");
    const QRegularExpression filterRegex = QRegularExpression(R"(^(?:ff|fire\w*)(?: (.+))$)", QRegularExpression::CaseInsensitiveOption);
    const QRegularExpression privateWindowFlagRegex = QRegularExpression(R"((\s+-p\b))", QRegularExpression::CaseInsensitiveOption);

    QString launchCommand;
    QList<Profile> profiles;
    bool hideDefaultProfile, showAlwaysPrivateWindows;
    QString firefoxIcon;
    const QString firefoxPrivateWindowIcon;
    bool privateWindowsAsActions;
#if KRUNNER_VERSION_MAJOR == 5
    QList<QAction *> matchActions;
#else
    KRunner::Actions matchActions;
#endif

    QList<QueryMatch> createProfileMatches(const QString &filter, bool privateWindow);
    QueryMatch createMatch(const QString &text, const QMap<QString, QVariant> &data, float relevance);

public: // AbstractRunner API
    void reloadConfiguration() override;
    void match(RunnerContext &context) override;
    void run(const RunnerContext &context, const QueryMatch &match) override;
};
