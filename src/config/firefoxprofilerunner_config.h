#ifndef FirefoxProfileRunnerCONFIG_H
#define FirefoxProfileRunnerCONFIG_H

#include "ui_firefoxprofilerunner_config.h"
#include <KCModule>
#include <KConfigCore/KConfigGroup>

class FirefoxProfileRunnerConfigForm : public QWidget, public Ui::FirefoxProfileRunnerConfigUi {
Q_OBJECT

public:
    explicit FirefoxProfileRunnerConfigForm(QWidget *parent);
};

class FirefoxProfileRunnerConfig : public KCModule {
Q_OBJECT

public:
    explicit FirefoxProfileRunnerConfig(QWidget *parent = nullptr, const QVariantList &args = QVariantList());

public Q_SLOTS:
    void save() override;
    void load() override;
    void defaults() override;

private:
    FirefoxProfileRunnerConfigForm *m_ui;

};

#endif
