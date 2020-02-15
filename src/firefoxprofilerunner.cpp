#include "firefoxprofilerunner.h"
#include "helper.h"
#include "profile/Profile.h"
#include <KLocalizedString>
#include <QDebug>
#include <QtCore/QProcess>
#include <QtCore/QFile>
#include <QtCore/QDir>

/**
 * TODO Make class names shorter
 * TODO Remove edited property
 * TODO Externalize config keys/constant data
 * TODO Improve debugging output
 */
FirefoxProfileRunner::FirefoxProfileRunner(QObject *parent, const QVariantList &args)
        : Plasma::AbstractRunner(parent, args) {
    setObjectName(QStringLiteral("FirefoxProfileRunner"));

    const QString configFolder = QDir::homePath() + "/.config/krunnerplugins/";
    const QDir configDir(configFolder);
    if (!configDir.exists()) configDir.mkpath(configFolder);
    // Create file
    QFile configFile(configFolder + "firefoxprofilerunnerrc");
    if (!configFile.exists()) {
        configFile.open(QIODevice::WriteOnly);
        configFile.close();
    }
    // Add file watcher for config
    watcher.addPath(configFolder + "firefoxprofilerunnerrc");
    connect(&watcher, &QFileSystemWatcher::fileChanged, this, &FirefoxProfileRunner::reloadPluginConfiguration);
    reloadPluginConfiguration();
}

void FirefoxProfileRunner::reloadPluginConfiguration(const QString &configFile) {
#ifdef status_dev
    qInfo() << "Firefox reload config";
#endif
    auto profileManager = ProfileManager();
    profiles = profileManager.syncAndGetCustomProfiles();
    if (profiles.isEmpty()) {
        // If the profiles.ini file has not changes and there are no profiles
        // For instance if you rerun the install script
        profiles = profileManager.syncAndGetCustomProfiles(true);
    }
    launchCommand = profileManager.launchCommand;
    firefoxIcon = QIcon::fromTheme(launchCommand.endsWith("firefox-esr") ? "firefox-esr" : "firefox");

    KConfigGroup config = KSharedConfig::openConfig(QDir::homePath() + "/.config/krunnerplugins/firefoxprofilerunnerrc")
            ->group("Config");
    if (!configFile.isEmpty()) config.config()->reparseConfiguration();

    // If the file gets edited with a text editor, it often gets replaced by the edited version
    // https://stackoverflow.com/a/30076119/9342842
    if (!configFile.isEmpty()) {
        if (QFile::exists(configFile)) {
            watcher.addPath(configFile);
        }
    }

    hideDefaultProfile = config.readEntry("hideDefaultProfile", false);
    showAlwaysPrivateWindows = config.readEntry("showAlwaysPrivateWindows", true);
    proxychainsForceNewInstance = config.readEntry("proxychainsForceNewInstance", false);
    proxychainsIntegrated = config.readEntry("proxychainsIntegration", "disabled") != "disabled";

    QList<Plasma::RunnerSyntax> syntaxes;
    syntaxes.append(Plasma::RunnerSyntax("firefox :q?",
                                         "Plugin gets triggered by firef... after that you can search the profiles by name")
    );
    syntaxes.append(Plasma::RunnerSyntax("firefox :q -p", "Launch profile in private window"));
    setSyntaxes(syntaxes);
}

void FirefoxProfileRunner::match(Plasma::RunnerContext &context) {
    QString term = context.query();
    if (!context.isValid() || !term.startsWith(prefix)) {
        return;
    }

    QList<Plasma::QueryMatch> matches;
    bool privateWindow = false;
    if (term.contains(privateWindowFlagRegex)) {
        privateWindow = true;
        term.remove(privateWindowFlagRegex);
    }

    const QString filter = filterRegex.match(term).captured(1);

    // Create matches and pass in value of private window flag
    matches.append(createProfileMatches(filter, privateWindow));
    // If private window flag is not set and private windows should always be shown create matches
    if (!privateWindow && showAlwaysPrivateWindows) matches.append(createProfileMatches(filter, true));

    context.addMatches(matches);
}

void FirefoxProfileRunner::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match) {
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
        if (proxychainsForceNewInstance) args.append("--new-instance");
    }
    if (data.contains("private-window")) args.append("-private-window");

    QProcess::startDetached(localLaunchCommand, args);
}

Plasma::QueryMatch
FirefoxProfileRunner::createMatch(const QString &text, const QMap<QString, QVariant> &data, float relevance) {
    Plasma::QueryMatch match(this);
    match.setIcon(data.contains("private-window") ? firefoxPrivateWindowIcon : firefoxIcon);
#ifdef status_dev
    match.setText(text + " " + QString::number(relevance));
#else
    match.setText(text);
#endif
    match.setData(data);
    match.setRelevance(relevance);
    return match;
}

QList<Plasma::QueryMatch> FirefoxProfileRunner::createProfileMatches(const QString &filter, const bool privateWindow) {
    QList<Plasma::QueryMatch> matches;
    for (const auto &profile: qAsConst(profiles)) {
        if (profile.name.startsWith(filter, Qt::CaseInsensitive)) {
            QMap<QString, QVariant> data;
            bool skipMatch = false;

            data.insert("name", profile.launchName);
            if (privateWindow) data.insert("private-window", "true");
            // Hide default profile
            if (profile.isDefault && hideDefaultProfile && !privateWindow) {
                if (!profile.extraNormalWindowProxychainsLaunchOption) continue;
                skipMatch = true;
            }
            const QString defaultNote = profile.isDefault ? " (default)" : "";
            const QString text = privateWindow ? "Private Window " + profile.name + defaultNote : profile.name +
                                                                                                  defaultNote;
            float priority = (float) profile.priority / 100;
            if (privateWindow && profile.privateWindowPriority != 0) {
                priority = (float) profile.privateWindowPriority / 100;
            } else if (privateWindow) {
                priority = (float) profile.priority / 101;
            }
            if (proxychainsIntegrated) {
                if (!privateWindow && profile.launchNormalWindowWithProxychains) {
                    data.insert("proxychains", true);
                } else if (privateWindow && profile.launchPrivateWindowWithProxychains) {
                    data.insert("proxychains", true);
                } else if (!privateWindow && profile.extraNormalWindowProxychainsLaunchOption) {
                    QMap<QString, QVariant> extraData(data);
                    extraData.insert("proxychains", true);
                    matches.append(createMatch("Proxychains: " + text, extraData,
                                               (float) profile.extraNormalWindowProxychainsOptionPriority / 100));
                } else if (privateWindow && profile.extraPrivateWindowProxychainsLaunchOption) {
                    QMap<QString, QVariant> extraData(data);
                    extraData.insert("proxychains", true);
                    matches.append(createMatch("Proxychains: " + text, extraData,
                                               (float) profile.extraPrivateWindowProxychainsOptionPriority / 100));
                }
            }
            if (!skipMatch) matches.append(createMatch(text, data, priority));
        }
    }
    return matches;
}


K_EXPORT_PLASMA_RUNNER(firefoxprofilerunner, FirefoxProfileRunner)

// needed for the QObject subclass declared as part of K_EXPORT_PLASMA_RUNNER
#include "firefoxprofilerunner.moc"
