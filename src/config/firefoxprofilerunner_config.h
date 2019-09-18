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

    QIcon firefoxIcon, firefoxPrivateWindowIcon;
    ProfileManager profileManager;
    QList<Profile> profiles;
    KSharedConfigPtr firefoxConfig;
    KConfigGroup config;

    QString previousProxychainsSelection;
    bool edited, forceProfileSync, proxychainsInstalled = false;

public Q_SLOTS:

    // KCModule slots
    void save() override;

    void load() override;

    void defaults() override;

    // General config slots
    void showAlwaysPrivateWindows();

    void hideDefaultProfile();

    void refreshProfiles();

    // Profile sorting/editing signals
    void itemSelected();

    void moveUp();

    void moveDown();

    void applyProfileName();

    void cancelProfileName();

    void editProfileName();

    // Proxychains config slots
    void loadInitialSettings(const QMap<QListWidgetItem *, Profile>& itemProfileMap);

    void proxychainsSelectionChanged();

    void validateProxychainsOptions();

    void toggleGeneralConfigVisibility(const QString &forceHide = "");

    void toggleProxychainsConfigVisibility(const QString &forceHide = "");

    void validateExtraOptionButtons();

    void addExtraOption();

    void removeExtraOption();


private:
    FirefoxProfileRunnerConfigForm *m_ui;

};

#endif
