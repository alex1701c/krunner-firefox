#include "firefoxprofilerunner.h"
#include "Profile.h"

// KF
#include <KLocalizedString>
#include <QDebug>
#include <QtCore/QProcess>

FirefoxProfileRunner::FirefoxProfileRunner(QObject *parent, const QVariantList &args)
        : Plasma::AbstractRunner(parent, args) {
    setObjectName(QStringLiteral("FirefoxProfileRunner"));
}

void FirefoxProfileRunner::init() {
    reloadConfiguration();
    connect(this, SIGNAL(prepare()), this, SLOT(prepareForMatchSession()));
    connect(this, SIGNAL(teardown()), this, SLOT(matchSessionFinished()));
}

void FirefoxProfileRunner::prepareForMatchSession() {
}

void FirefoxProfileRunner::matchSessionFinished() {
}

FirefoxProfileRunner::~FirefoxProfileRunner() = default;

void FirefoxProfileRunner::reloadConfiguration() {

    profiles = Profile::getProfiles();
    /*for (const auto &p:profiles) {
        qInfo() << p.name << p.path << p.isDefault ;
    }*/
    Profile::syncDesktopFile(profiles);

    QList<Plasma::RunnerSyntax> syntaxes;
    syntaxes.append(Plasma::RunnerSyntax("query", "Explain query"));
    setSyntaxes(syntaxes);
}

void FirefoxProfileRunner::match(Plasma::RunnerContext &context) {
    if (!context.isValid()) return;
    QString term = context.query();
    if (!term.startsWith("fire")) return;
    QList<Plasma::QueryMatch> matches;
    bool privateWindow = false;
    if (term.endsWith(" -p")) {
        privateWindow = true;
        term.remove(QRegExp(" -p$"));
    }
    QRegExp regExp(R"(^fire\w*(?: (.+))$)");
    regExp.indexIn(term);
    QString filter = regExp.capturedTexts().last();
    for (auto &profile:profiles) {
        qInfo() << filter << privateWindow;
        if (profile.name.startsWith(filter, Qt::CaseInsensitive)) {
            QMap<QString, QVariant> data;
            data.insert("name", profile.name);
            data.insert("private-window", privateWindow ? "true" : "");

            matches.append(createMatch(privateWindow ? "Private Window " + profile.name : profile.name, data, 1));
        }
    }

    context.addMatches(matches);
}

void FirefoxProfileRunner::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match) {
    Q_UNUSED(context)

    const QMap<QString, QVariant> data = match.data().toMap();
    QStringList args = {"-P", data.value("name").toString()};
    if (!data.value("private-window").toString().isEmpty()) {
        args.append("-private-window");
    }

    QProcess::startDetached("firefox", args);
}

Plasma::QueryMatch
FirefoxProfileRunner::createMatch(const QString &text, const QMap<QString, QVariant> &data, float relevance) {
    Plasma::QueryMatch match(this);
    match.setIconName("firefox");
    match.setText(text);
    match.setData(data);
    match.setRelevance(relevance);
    return match;
}


K_EXPORT_PLASMA_RUNNER(firefoxprofilerunner, FirefoxProfileRunner)

// needed for the QObject subclass declared as part of K_EXPORT_PLASMA_RUNNER
#include "firefoxprofilerunner.moc"
