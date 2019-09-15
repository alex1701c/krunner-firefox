#ifndef FirefoxProfileRunnerCONFIG_H
#define FirefoxProfileRunnerCONFIG_H

#include "ui_firefoxprofilerunner_config.h"
#include <KCModule>
#include <KConfigCore/KConfigGroup>
#include "profile/ProfileManager.h"

class FirefoxProfileRunnerConfigForm : public QWidget, public Ui::FirefoxProfileRunnerConfigUi {
Q_OBJECT

public:
    explicit FirefoxProfileRunnerConfigForm(QWidget *parent);
};

class FirefoxProfileRunnerConfig : public KCModule {
Q_OBJECT

public:
    explicit FirefoxProfileRunnerConfig(QWidget *parent = nullptr, const QVariantList &args = QVariantList());

    ProfileManager profileManager;
    QList<Profile> profiles;
    KSharedConfigPtr firefoxConfig;
    KConfigGroup config;

    bool edited, forceProfileSync, proxychainsInstalled = false;

public Q_SLOTS:


    void save() override;

    void load() override;

    void defaults() override;

    void itemSelected();

    void moveUp();

    void moveDown();

    void applyProfileName();

    void cancelProfileName();

    void editProfileName();

    void refreshProfiles();

    void hideDefaultProfile();

    void showAlwaysPrivateWindows();


    void validateProxychainsOptions();

    void toggleGeneralConfigVisibility(const QString &forceHide = "");

    void toggleProxychainsConfigVisibility(const QString &forceHide = "");


private:
    FirefoxProfileRunnerConfigForm *m_ui;

};

#endif
