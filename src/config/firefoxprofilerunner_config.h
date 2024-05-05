#ifndef FirefoxProfileRunnerCONFIG_H
#define FirefoxProfileRunnerCONFIG_H

#include <QHash>

#include "profile/ProfileManager.h"
#include "ui_firefoxprofilerunner_config.h"
#include <KCModule>
#include <KConfigGroup>
#include <QIcon>

class FirefoxProfileRunnerConfigForm : public QWidget, public Ui::FirefoxProfileRunnerConfigUi
{
    Q_OBJECT

public:
    explicit FirefoxProfileRunnerConfigForm(QWidget *parent);
};

class FirefoxRunnerConfig : public KCModule
{
    Q_OBJECT

public:
    explicit FirefoxRunnerConfig(QObject *parent, const QVariantList &args);

    QIcon firefoxIcon;
    const QIcon firefoxPrivateWindowIcon = QIcon::fromTheme("private_browsing_firefox");
    const QIcon upIcon = QIcon::fromTheme("arrow-up");
    const QIcon downIcon = QIcon::fromTheme("arrow-down");
    ProfileManager profileManager;
    QList<Profile> profiles;
    KSharedConfigPtr firefoxConfig;
    KConfigGroup config;

    bool edited, forceProfileSync = false;

public Q_SLOTS:
    void save() override;
    void load() override;
    void defaults() override;
    void connectSignals();

    // General config slots
    void showAlwaysPrivateWindows();
    void hideDefaultProfile();
    void refreshProfiles();
    void privateWindowsAsActionsChanged();

    // Profile sorting/editing signals
    void itemSelected();
    void moveUp();
    void moveDown();

    void applyProfileName();
    void cancelProfileName();
    void editProfileName();
    void toggleGeneralConfigVisibility();

private:
    FirefoxProfileRunnerConfigForm *m_ui;
};

#endif
