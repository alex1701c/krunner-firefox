#ifndef FirefoxProfileRunnerCONFIG_H
#define FirefoxProfileRunnerCONFIG_H

#include "ui_firefoxprofilerunner_config.h"
#include <KCModule>
#include <KConfigCore/KConfigGroup>
#include <Profile.h>

class FirefoxProfileRunnerConfigForm : public QWidget, public Ui::FirefoxProfileRunnerConfigUi {
Q_OBJECT

public:
    explicit FirefoxProfileRunnerConfigForm(QWidget *parent);
};

class FirefoxProfileRunnerConfig : public KCModule {
Q_OBJECT

public:
    explicit FirefoxProfileRunnerConfig(QWidget *parent = nullptr, const QVariantList &args = QVariantList());

    QList<Profile> profiles;

public Q_SLOTS:


    void save() override;

    void load() override;

    void itemSelected();
    void moveUp();
    void moveDown();
private:
    FirefoxProfileRunnerConfigForm *m_ui;

};

#endif
