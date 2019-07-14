#ifndef FIREFOXPROFILERUNNER_H
#define FIREFOXPROFILERUNNER_H

#include <KRunner/AbstractRunner>

class FirefoxProfileRunner : public Plasma::AbstractRunner
{
    Q_OBJECT

public:
    FirefoxProfileRunner(QObject *parent, const QVariantList &args);
    ~FirefoxProfileRunner() override;

protected Q_SLOTS:
    void init() override;
    void prepareForMatchSession();
    void matchSessionFinished();

public: // Plasma::AbstractRunner API
    void match(Plasma::RunnerContext &context) override;
    void run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match) override;
    void reloadConfiguration() override;
};

#endif
