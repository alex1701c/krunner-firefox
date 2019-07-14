#include "firefoxprofilerunner.h"
#include "Profile.h"

// KF
#include <KLocalizedString>
#include <QDebug>

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
    const QString term = context.query();
    if (term.length() < 3) {
        return;
    }
    QList<Plasma::QueryMatch> matches;

    Plasma::QueryMatch match(this);
    match.setIconName("kdeapp");
    match.setText("Hello World!");
    matches.append(match);

    context.addMatches(matches);
}

void FirefoxProfileRunner::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match) {
    Q_UNUSED(context)
    Q_UNUSED(match)

    // TODO
}

K_EXPORT_PLASMA_RUNNER(firefoxprofilerunner, FirefoxProfileRunner)

// needed for the QObject subclass declared as part of K_EXPORT_PLASMA_RUNNER
#include "firefoxprofilerunner.moc"
