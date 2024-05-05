#include "firefoxprofilerunner.h"
#include "Config.h"
#include "profile/Profile.h"
#include "profile/ProfileManager.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSycoca>
#include <QAction>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QProcess>

FirefoxRunner::FirefoxRunner(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
#if KRUNNER_VERSION_MAJOR == 5
    : AbstractRunner(parent, data, args)
    , matchActions({new QAction(firefoxPrivateWindowIcon, "Open profile in private window", this)})
#else
    : AbstractRunner(parent, data)
    , matchActions({KRunner::Action("private-window", firefoxPrivateWindowIcon.name(), "Open profile in private window")})
#endif
{
    Q_UNUSED(args)
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
#if KRUNNER_VERSION_MAJOR == 5
    match.setType(QueryMatch::ExactMatch);
#else
    match.setCategoryRelevance(QueryMatch::CategoryRelevance::Highest);
#endif
    if (privateWindowsAsActions) {
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
            data.insert("name", profile.launchName);
            if (privateWindow)
                data.insert("private-window", "true");
            // Hide default profile
            if (profile.isDefault && hideDefaultProfile && !privateWindow) {
                continue;
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

            matches.append(createMatch(text, data, priority));
        }
    }
    return matches;
}

K_PLUGIN_CLASS_WITH_JSON(FirefoxRunner, "firefoxprofilerunner.json")

#include "firefoxprofilerunner.moc"
#include "moc_firefoxprofilerunner.cpp"
