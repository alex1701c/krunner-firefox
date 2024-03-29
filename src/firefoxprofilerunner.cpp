#include "firefoxprofilerunner.h"
#include "profile/Profile.h"
#include <KLocalizedString>
#include <KSycoca>
#include <QAction>
#include <QDebug>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QProcess>

#include "Config.h"

FirefoxRunner::FirefoxRunner(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
    : AbstractRunner(parent, data, args)
    , matchActions({new QAction(firefoxPrivateWindowIcon, "Open profile in private window", this)})
{
    setObjectName(QStringLiteral("FirefoxProfileRunner"));
    filterRegex.optimize();
    privateWindowFlagRegex.optimize();
}

void FirefoxRunner::reloadConfiguration()
{
    ProfileManager profileManager;
    profiles = profileManager.syncAndGetCustomProfiles();
    if (profiles.isEmpty()) {
        // If the profiles.ini file has not changes and there are no profiles
        // For instance if you rerun the install script
        profiles = profileManager.syncAndGetCustomProfiles(true);
    }
    launchCommand = profileManager.launchCommand;
    firefoxIcon = QIcon::fromTheme(launchCommand.endsWith("firefox-esr") ? "firefox-esr" : "firefox");

    KConfigGroup config = KSharedConfig::openConfig(Config::ConfigFile)->group(Config::MainGroup);
    config.config()->reparseConfiguration();

    hideDefaultProfile = config.readEntry(Config::HideDefaultProfile, false);
    showAlwaysPrivateWindows = config.readEntry(Config::ShowAlwaysPrivateWindows, true);
    proxychainsForceNewInstance = config.readEntry(Config::ProxychainsForceNewInstance, false);
    proxychainsIntegrated = config.readEntry(Config::ProxychainsIntegration, "disabled") != "disabled";

    privateWindowsAsActions = config.readEntry(Config::PrivateWindowAction, false);

    QList<RunnerSyntax> syntaxes;
    syntaxes.append(RunnerSyntax("firefox :q:", "Plugin gets triggered by fire... after that you can search the profiles by name"));
    syntaxes.append(RunnerSyntax("firefox -p :q:", "Launch profile in private window"));
    syntaxes.append(RunnerSyntax("firefox :q: -p", "Launch profile in private window"));
    syntaxes.append(RunnerSyntax("ff :q:", "Plugin gets triggered by ff... after that you can search the profiles by name"));
    syntaxes.append(RunnerSyntax("ff -p :q:", "Launch profile in private window"));
    syntaxes.append(RunnerSyntax("ff :q: -p", "Launch profile in private window"));
    setSyntaxes(syntaxes);
}

void FirefoxRunner::match(RunnerContext &context)
{
    KSycoca::disableAutoRebuild();
    QString term = context.query();
    if (!context.isValid()) {
        return;
    }
    if (!term.startsWith(shortPrefix, Qt::CaseInsensitive) && !term.startsWith(mediumPrefix, Qt::CaseInsensitive)) {
        return;
    }

    QList<QueryMatch> matches;
    bool privateWindow = false;
    if (!privateWindowsAsActions && term.contains(privateWindowFlagRegex)) {
        privateWindow = true;
        term.remove(privateWindowFlagRegex);
    }

    const QString filter = filterRegex.match(term).captured(1);

    // Create matches and pass in value of private window flag
    matches.append(createProfileMatches(filter, privateWindow));

    // If private window flag is not set and private windows should always be shown create matches
    if (!privateWindow && showAlwaysPrivateWindows) {
        matches.append(createProfileMatches(filter, true));
    }

    context.addMatches(matches);
}

void FirefoxRunner::run(const RunnerContext &context, const QueryMatch &match)
{
    Q_UNUSED(context)
    const QMap<QString, QVariant> data = match.data().toMap();
    QStringList args = {"-P", data.value("name").toString()};
    QString localLaunchCommand = launchCommand;
#ifdef status_dev
    qInfo() << data;
#endif
    if (data.contains("proxychains")) {
        args.prepend(localLaunchCommand);
        args.prepend("-q");
        localLaunchCommand = "proxychains4";
        if (proxychainsForceNewInstance) {
            args.append("--new-instance");
        }
    }
    // Private window if data has private window key or the action exists
    if (data.contains("private-window") || match.selectedAction()) {
        args.append("-private-window");
    }

    QProcess::startDetached(localLaunchCommand, args);
}

QueryMatch FirefoxRunner::createMatch(const QString &text, const QMap<QString, QVariant> &data, float relevance)
{
    QueryMatch match(this);
    match.setIcon(data.contains("private-window") ? firefoxPrivateWindowIcon : firefoxIcon);
    match.setText(text);
    match.setData(data);
    match.setRelevance(relevance);
    if (privateWindowsAsActions && !match.text().startsWith(proxychainsDisplayPrefix)) {
        match.setActions(matchActions);
    }
    return match;
}

QList<QueryMatch> FirefoxRunner::createProfileMatches(const QString &filter, const bool privateWindow)
{
    QList<::QueryMatch> matches;
    for (const auto &profile : qAsConst(profiles)) {
        if (profile.name.startsWith(filter, Qt::CaseInsensitive)) {
            QMap<QString, QVariant> data;
            bool skipMatch = false;

            data.insert("name", profile.launchName);
            if (privateWindow)
                data.insert("private-window", "true");
            // Hide default profile
            if (profile.isDefault && hideDefaultProfile && !privateWindow) {
                if (!profile.extraNormalWindowProxychainsLaunchOption)
                    continue;
                skipMatch = true;
            }
            QString text = profile.name;
            if (privateWindow)
                text.prepend("Private Window ");
            if (profile.isDefault)
                text.append(" (default)");

            float priority = (float)profile.priority / 100;
            if (privateWindow && profile.privateWindowPriority != 0) {
                priority = (float)profile.privateWindowPriority / 100;
            } else if (privateWindow) {
                priority = (float)profile.priority / 101;
            }

            if (proxychainsIntegrated) {
                if (!privateWindow && profile.launchNormalWindowWithProxychains) {
                    data.insert("proxychains", true);
                } else if (privateWindow && profile.launchPrivateWindowWithProxychains) {
                    data.insert("proxychains", true);
                } else if (!privateWindow && profile.extraNormalWindowProxychainsLaunchOption) {
                    QMap<QString, QVariant> extraData(data);
                    extraData.insert("proxychains", true);
                    matches.append(createMatch(proxychainsDisplayPrefix + text, extraData, (float)profile.extraNormalWindowProxychainsOptionPriority / 100));
                } else if ((privateWindowsAsActions || privateWindow) && profile.extraPrivateWindowProxychainsLaunchOption) {
                    QMap<QString, QVariant> extraData(data);
                    extraData.insert("proxychains", true);
                    extraData.insert("private-window", true);
                    matches.append(createMatch(proxychainsDisplayPrefix + text, extraData, (float)profile.extraPrivateWindowProxychainsOptionPriority / 100));
                }
            }
            if (!skipMatch)
                matches.append(createMatch(text, data, priority));
        }
    }
    return matches;
}

K_PLUGIN_CLASS_WITH_JSON(FirefoxRunner, "firefoxprofilerunner.json")

// needed for the QObject subclass declared as part of K_EXPORT_PLASMA_RUNNER
#include "firefoxprofilerunner.moc"
