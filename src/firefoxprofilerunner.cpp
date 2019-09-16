#include "firefoxprofilerunner.h"
#include "helper.h"
#include "profile/Profile.h"
#include <KLocalizedString>
#include <QDebug>
#include <QtCore/QProcess>

FirefoxProfileRunner::FirefoxProfileRunner(QObject *parent, const QVariantList &args)
        : Plasma::AbstractRunner(parent, args) {
    setObjectName(QStringLiteral("FirefoxProfileRunner"));
}

void FirefoxProfileRunner::reloadConfiguration() {
    profileManager = ProfileManager();
    profiles = profileManager.syncAndGetCustomProfiles();
    launchCommand = profileManager.getLaunchCommand();

    firefoxIcon = QIcon::fromTheme(launchCommand.endsWith("firefox-esr") ? "firefox-esr" : "firefox");
    firefoxPrivateWindowIcon = QIcon("/usr/share/icons/private_browsing_firefox.svg");

    config = KSharedConfig::openConfig("krunnerrc")->group("Runners").group("FirefoxProfileRunner");
    hideDefaultProfile = stringToBool(config.readEntry("hideDefaultProfile"));
    showAlwaysPrivateWindows = stringToBool(config.readEntry("showAlwaysPrivateWindows", "true"));

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

    matches.append(createProfileMatches(filter, privateWindow));
    if (!privateWindow && showAlwaysPrivateWindows) matches.append(createProfileMatches(filter, true));

    context.addMatches(matches);
}

void FirefoxProfileRunner::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match) {
    Q_UNUSED(context)

    const QMap<QString, QVariant> data = match.data().toMap();
    QStringList args = {"-P", data.value("name").toString()};
    if (data.count("private-window")) args.append("-private-window");

    QProcess::startDetached(launchCommand, args);
}

Plasma::QueryMatch
FirefoxProfileRunner::createMatch(const QString &text, const QMap<QString, QVariant> &data, float relevance) {
    Plasma::QueryMatch match(this);
    match.setIcon(data.count("private-window") ? firefoxPrivateWindowIcon : firefoxIcon);
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
            data.insert("name", profile.launchName);
            if (privateWindow) data.insert("private-window", "true");
            if (profile.isDefault && hideDefaultProfile && !privateWindow) continue;
            QString defaultNote = profile.isDefault ? " (default)" : "";
            QString text = privateWindow ? "Private Window " + profile.name + defaultNote : profile.name + defaultNote;
            float priority = (float) profile.priority / 100;
            if (privateWindow && profile.privateWindowPriority != 0) {
                priority = (float) profile.privateWindowPriority / 100;
            } else if (privateWindow) {
                priority = (float) profile.priority / 101;
            }
            matches.append(createMatch(text, data, priority));
        }
    }
    return matches;
}


K_EXPORT_PLASMA_RUNNER(firefoxprofilerunner, FirefoxProfileRunner)

// needed for the QObject subclass declared as part of K_EXPORT_PLASMA_RUNNER
#include "firefoxprofilerunner.moc"
