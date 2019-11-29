#include "firefoxprofilerunner.h"
#include "helper.h"
#include "profile/Profile.h"
#include <KLocalizedString>
#include <QDebug>
#include <QtCore/QProcess>
#include <QtCore/QFile>

FirefoxProfileRunner::FirefoxProfileRunner(QObject *parent, const QVariantList &args)
        : Plasma::AbstractRunner(parent, args) {
    setObjectName(QStringLiteral("FirefoxProfileRunner"));
}

void FirefoxProfileRunner::reloadConfiguration() {
    profileManager = ProfileManager();
    profiles = profileManager.syncAndGetCustomProfiles();
    launchCommand = profileManager.launchCommand;
    firefoxIcon = QIcon::fromTheme(launchCommand.endsWith("firefox-esr") ? "firefox-esr" : "firefox");

    config = KSharedConfig::openConfig("krunnerrc")->group("Runners").group("FirefoxProfileRunner");
    hideDefaultProfile = stringToBool(config.readEntry("hideDefaultProfile"));
    showAlwaysPrivateWindows = stringToBool(config.readEntry("showAlwaysPrivateWindows", "true"));
    proxychainsForceNewInstance = stringToBool(config.readEntry("proxychainsForceNewInstance"));
    proxychainsIntegrated = config.readEntry("proxychainsIntegration", "disabled") != "disabled";

#ifdef status_dev
    for (const auto &p:profiles) {
        qInfo() << "Name: " << p.name << "Launch Name: " << p.launchName << "Path: " << p.path
                << "Is Default: " << p.isDefault << "Priority: " << p.priority << "Edited: " << p.isEdited;
    }
#endif

    QList<Plasma::RunnerSyntax> syntaxes;
    syntaxes.append(
            Plasma::RunnerSyntax("firefox :q?",
                                 "Plugin gets triggered by firef... after that you can search the profiles by name")
    );
    syntaxes.append(Plasma::RunnerSyntax("firefox :q -p", "Launch profile in private window"));
    setSyntaxes(syntaxes);
}

void FirefoxProfileRunner::match(Plasma::RunnerContext &context) {
    if (!context.isValid()) return;
    QString term = context.query();
    if (!term.startsWith("fire")) return;

    QList<Plasma::QueryMatch> matches;
    bool privateWindow = false;
    if (term.contains(QRegExp(" -p *$"))) {
        privateWindow = true;
        term.remove(QRegExp(" -p *$"));
    }

    QRegExp regExp(R"(^fire\w*(?: (.+))$)");
    regExp.indexIn(term);
    QString filter = regExp.capturedTexts().last();

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
    //qInfo() << data;
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
    match.setText(text);
    match.setData(data);
    match.setRelevance(relevance);
    return match;
}

QList<Plasma::QueryMatch> FirefoxProfileRunner::createProfileMatches(const QString &filter, const bool privateWindow) {
    QList<Plasma::QueryMatch> matches;
    for (auto &profile:profiles) {
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
            QString defaultNote = profile.isDefault ? " (default)" : "";
            QString text = privateWindow ? "Private Window " + profile.name + defaultNote : profile.name + defaultNote;
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
