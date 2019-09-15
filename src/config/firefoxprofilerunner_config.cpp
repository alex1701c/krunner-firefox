#include "firefoxprofilerunner_config.h"
#include <KSharedConfig>
#include <KPluginFactory>
#include <krunner/abstractrunner.h>
#include <QtWidgets/QTableWidgetItem>
#include <QtWidgets/QCheckBox>
#include <QDebug>
#include <QtCore/QDir>
#include <QtWidgets/QMessageBox>
#include "helper.h"
/**
 * TODO Integrate picker to create new profiles with proxychains
 * TODO Save new profiles
 * TODO Option to create Desktop Action with proxychains
 * TODO Launch proxychains from config
 */
K_PLUGIN_FACTORY(FirefoxProfileRunnerConfigFactory,
                 registerPlugin<FirefoxProfileRunnerConfig>("kcm_krunner_firefoxprofilerunner");)

FirefoxProfileRunnerConfigForm::FirefoxProfileRunnerConfigForm(QWidget *parent) : QWidget(parent) {
    setupUi(this);
}

FirefoxProfileRunnerConfig::FirefoxProfileRunnerConfig(QWidget *parent, const QVariantList &args) : KCModule(parent, args) {
    m_ui = new FirefoxProfileRunnerConfigForm(this);
    auto *layout = new QGridLayout(this);
    layout->addWidget(m_ui, 0, 0);

    // General settings
    connect(m_ui->registerNormalWindows, SIGNAL(clicked(bool)), this, SLOT(changed()));
    connect(m_ui->registerPrivateWindows, SIGNAL(clicked(bool)), this, SLOT(changed()));
    connect(m_ui->automaticallyRegisterProfiles, SIGNAL(clicked(bool)), this, SLOT(changed()));
    connect(m_ui->hideDefaultProfile, SIGNAL(clicked(bool)), this, SLOT(changed()));
    connect(m_ui->hideDefaultProfile, SIGNAL(clicked(bool)), this, SLOT(hideDefaultProfile()));
    connect(m_ui->hideDefaultProfile, SIGNAL(clicked(bool)), this, SLOT(itemSelected()));
    connect(m_ui->showIconForPrivateWindow, SIGNAL(clicked(bool)), this, SLOT(changed()));
    connect(m_ui->showAlwaysPrivateWindows, SIGNAL(clicked(bool)), this, SLOT(changed()));
    connect(m_ui->showAlwaysPrivateWindows, SIGNAL(clicked(bool)), this, SLOT(showAlwaysPrivateWindows()));
    connect(m_ui->showAlwaysPrivateWindows, SIGNAL(clicked(bool)), this, SLOT(itemSelected()));
    // Different item gets selected
    connect(m_ui->profiles, SIGNAL(itemSelectionChanged()), this, SLOT(itemSelected()));
    connect(m_ui->profiles, SIGNAL(itemSelectionChanged()), this, SLOT(editProfileName()));
    // Order gets changes
    connect(m_ui->moveUp, SIGNAL(clicked(bool)), this, SLOT(moveUp()));
    connect(m_ui->moveDown, SIGNAL(clicked(bool)), this, SLOT(moveDown()));
    connect(m_ui->moveUp, SIGNAL(clicked(bool)), this, SLOT(changed()));
    connect(m_ui->moveDown, SIGNAL(clicked(bool)), this, SLOT(changed()));
    // Edit profile name
    connect(m_ui->editProfileName, SIGNAL(textChanged(QString)), this, SLOT(editProfileName()));
    connect(m_ui->editProfileNameApply, SIGNAL(clicked(bool)), this, SLOT(applyProfileName()));
    connect(m_ui->editProfileNameApply, SIGNAL(clicked(bool)), this, SLOT(changed()));
    connect(m_ui->editProfileNameCancel, SIGNAL(clicked(bool)), this, SLOT(cancelProfileName()));
    // Refresh profiles button
    connect(m_ui->refreshProfiles, SIGNAL(clicked(bool)), this, SLOT(refreshProfiles()));
    connect(m_ui->refreshProfiles, SIGNAL(clicked(bool)), this, SLOT(changed()));
    // Hide/unhide buttons
    connect(m_ui->generalConfigToggleHidePushButton, SIGNAL(clicked(bool)), this, SLOT(toggleGeneralConfigVisibility()));
    connect(m_ui->generalConfigToggleHidePushButton, SIGNAL(clicked(bool)), this, SLOT(changed()));
    connect(m_ui->proxychainsToggleHidePushButton, SIGNAL(clicked(bool)), this, SLOT(toggleProxychainsConfigVisibility()));
    connect(m_ui->proxychainsToggleHidePushButton, SIGNAL(clicked(bool)), this, SLOT(changed()));
    // Proxychains options changed
    connect(m_ui->disableProxychainsRadioButton, SIGNAL(clicked(bool)), this, SLOT(changed()));
    connect(m_ui->launchExistingOptionRadioButton, SIGNAL(clicked(bool)), this, SLOT(changed()));
    connect(m_ui->showExtraOptionRadioButton, SIGNAL(clicked(bool)), this, SLOT(changed()));
    connect(m_ui->forceNewInstanceCheckBox, SIGNAL(clicked(bool)), this, SLOT(changed()));
    //
    connect(m_ui->disableProxychainsRadioButton, SIGNAL(clicked(bool)), this, SLOT(validateProxychainsOptions()));
    connect(m_ui->launchExistingOptionRadioButton, SIGNAL(clicked(bool)), this, SLOT(validateProxychainsOptions()));
    connect(m_ui->showExtraOptionRadioButton, SIGNAL(clicked(bool)), this, SLOT(validateProxychainsOptions()));
    connect(m_ui->disableProxychainsRadioButton, SIGNAL(clicked(bool)), this, SLOT(proxychainsSelectionChanged()));
    connect(m_ui->launchExistingOptionRadioButton, SIGNAL(clicked(bool)), this, SLOT(proxychainsSelectionChanged()));
    connect(m_ui->showExtraOptionRadioButton, SIGNAL(clicked(bool)), this, SLOT(proxychainsSelectionChanged()));

    firefoxConfig = KSharedConfig::openConfig(profileManager.firefoxDesktopFile);
    config = KSharedConfig::openConfig("krunnerrc")->group("Runners").group("FirefoxProfileRunner");
    proxychainsInstalled = QFile::exists("/usr/bin/proxychains4");
}

void FirefoxProfileRunnerConfig::load() {
    m_ui->automaticallyRegisterProfiles->setChecked(stringToBool(config.readEntry("automaticallyRegisterProfiles", "true")));
    m_ui->registerPrivateWindows->setChecked(stringToBool(config.readEntry("registerPrivateWindows")));
    m_ui->registerNormalWindows->setChecked(stringToBool(config.readEntry("registerNormalWindows", "true")));
    m_ui->showIconForPrivateWindow->setChecked(stringToBool(config.readEntry("showIconForPrivateWindow", "true")));
    m_ui->hideDefaultProfile->setChecked(stringToBool(config.readEntry("hideDefaultProfile")));
    m_ui->showAlwaysPrivateWindows->setChecked(stringToBool(config.readEntry("showAlwaysPrivateWindows")));

    profiles = profileManager.syncAndGetCustomProfiles(forceProfileSync);
    forceProfileSync = false;
    m_ui->profiles->clear();

    QList<QListWidgetItem *> items;
    const auto icon = QIcon::fromTheme(profileManager.launchCommand.endsWith("firefox") ? "firefox" : "firefox-esr");
    for (const auto &profile:profiles) {
        // Normal window
        auto *item = new QListWidgetItem();
        item->setText(profile.name);
        QList<QVariant> data = {profile.path, profile.isDefault, false, profile.priority};
        item->setData(32, data);
        item->setIcon(icon);
        items.append(item);
        // Private window
        auto *item2 = new QListWidgetItem();
        item2->setText(profile.name);
        QList<QVariant> data2 = {profile.path, profile.isDefault, true, profile.privateWindowPriority};
        item2->setData(32, data2);
        item2->setIcon(QIcon("/usr/share/icons/private_browsing_firefox.svg"));
        items.append(item2);
    }
    std::sort(items.begin(), items.end(), [](QListWidgetItem *item1, QListWidgetItem *item2) -> bool {
        return item1->data(32).toList().last() > item2->data(32).toList().last();
    });
    for (const auto &item:items) m_ui->profiles->addItem(item);

    // Initial validation
    showAlwaysPrivateWindows();
    hideDefaultProfile();
    toggleGeneralConfigVisibility(stringToBool(config.readEntry("generalMinimized")) ? "true" : "skip");
    if (proxychainsInstalled) {
        // Load data
        m_ui->forceNewInstanceCheckBox->setChecked(stringToBool(config.readEntry("proxychainsForceNewInstance")));
        const QString proxychainsChoice = config.readEntry("proxychainsIntegration", "disabled");
        previousProxychainsSelection = proxychainsChoice;
        if (proxychainsChoice == "disabled") m_ui->disableProxychainsRadioButton->setChecked(true);
        else if (proxychainsChoice == "existing") m_ui->launchExistingOptionRadioButton->setChecked(true);
        else m_ui->showExtraOptionRadioButton->setChecked(true);
        // Validate
        m_ui->proxychainsNotInstalledLabel->hide();
        toggleProxychainsConfigVisibility(stringToBool(config.readEntry("proxychainsMinimized")) ? "true" : "skip");
        validateProxychainsOptions();
    } else {
        m_ui->proxychainsConfigGroupBox->hide();
    }
    emit changed(false);
}


void FirefoxProfileRunnerConfig::save() {
    // Write general settings
    config.writeEntry("generalMinimized", m_ui->firefoxGeneralConfigWidget->isHidden());
    config.writeEntry("registerNormalWindows", m_ui->registerNormalWindows->isChecked());
    config.writeEntry("registerPrivateWindows", m_ui->registerPrivateWindows->isChecked());
    config.writeEntry("hideDefaultProfile", m_ui->hideDefaultProfile->isChecked());
    config.writeEntry("showIconForPrivateWindow", m_ui->showIconForPrivateWindow->isChecked());
    config.writeEntry("showAlwaysPrivateWindows", m_ui->showAlwaysPrivateWindows->isChecked());
    config.writeEntry("automaticallyRegisterProfiles", m_ui->automaticallyRegisterProfiles->isChecked());

    // Write proxychains settings
    if (proxychainsInstalled) {
        config.writeEntry("proxychainsMinimized", m_ui->proxychainsItemsWidget->isHidden());
        config.writeEntry("proxychainsForceNewInstance", m_ui->forceNewInstanceCheckBox->isChecked());
        QString proxychainsChoice;
        if (m_ui->disableProxychainsRadioButton->isChecked()) proxychainsChoice = "disabled";
        else if (m_ui->launchExistingOptionRadioButton) proxychainsChoice = "existing";
        else proxychainsChoice = "extra";
        config.writeEntry("proxychainsIntegration", proxychainsChoice);
    }
    QList<QListWidgetItem *> items;
    for (int i = 0; i < m_ui->profiles->count(); i++) {
        items.append(m_ui->profiles->item(i));
    }

    // Update settings in firefox.desktop file
    const int itemSize = items.size();
    for (auto &profile:profiles) {
        bool matched = false;
        for (int i = 0; i < itemSize && !matched; i++) {
            const auto &item = items.at(i);
            const auto itemData = item->data(32).toList();
            if (itemData.first() == profile.path && !itemData.at(2).toBool()) {
                if (profile.launchName != item->text()) profile.isEdited = true;
                profile.name = item->text();
                profile.priority = 100 - i;

                // Get private window option of profile
                bool privateMatch = false;
                for (int i2 = 0; i2 < itemSize && !privateMatch; i2++) {
                    const auto &data = items.at(i2)->data(32).toList();
                    if (data.first() == profile.path && data.at(2).toBool()) {
                        privateMatch = true;
                        profile.privateWindowPriority = 100 - i2;
                    }
                }
                profile.writeConfigChanges(firefoxConfig);
                matched = true;
            }
        }
    }
    // If the runner does not register the profiles on startup
    if (!m_ui->automaticallyRegisterProfiles->isChecked()) {
        profileManager.changeProfileRegistering(m_ui->registerNormalWindows->isChecked(), m_ui->registerPrivateWindows->isChecked(),
                                                firefoxConfig);
    }
    // New runner instance has latest configuration
    system("kquitapp5 krunner;kstart5 krunner > /dev/null 2&>1");

    emit changed(true);
}

void FirefoxProfileRunnerConfig::defaults() {
    m_ui->registerNormalWindows->setChecked(true);
    m_ui->registerPrivateWindows->setChecked(false);
    m_ui->automaticallyRegisterProfiles->setChecked(true);
    m_ui->hideDefaultProfile->setChecked(false);
    m_ui->showIconForPrivateWindow->setChecked(true);
    m_ui->showAlwaysPrivateWindows->setChecked(false);
}

/**
 * Enable/Disable Up/Down buttons
 */
void FirefoxProfileRunnerConfig::itemSelected() {
    const int idx = m_ui->profiles->currentRow();
    if (idx == -1) return; // When the checkboxes are clicked but no item has been selected
    const int count = m_ui->profiles->count();

    // Check if there is any hidden option below/above current item
    bool upDisabled = true;
    bool downDisabled = true;
    for (int i = 0; i < count; i++) {
        if (!m_ui->profiles->item(i)->isHidden() && i < idx) {
            upDisabled = false;
            break;
        }
    }
    for (int i = count - 1; i >= 0; i--) {
        if (!m_ui->profiles->item(i)->isHidden() && i > idx) {
            downDisabled = false;
            break;
        }
    }
    m_ui->moveUp->setDisabled(upDisabled);
    m_ui->moveDown->setDisabled(downDisabled);
    m_ui->editProfileName->setText(m_ui->profiles->currentItem()->text());
    m_ui->editProfileName->setDisabled(false);
}

/**
 * Refresh profiles from profiles.ini file, all unsaved changes are discarded
 */
void FirefoxProfileRunnerConfig::refreshProfiles() {
    if (!edited) {
        forceProfileSync = true;
        load();
    } else {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Discard changes?",
                                      "Do you want to refresh the current config and discard all unsaved changes",
                                      QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            forceProfileSync = true;
            load();
        } else return;
    }
    // Disable Buttons like on initial state
    m_ui->profiles->setFocus();
    if (m_ui->profiles->count() > 0) m_ui->profiles->setCurrentRow(0);
    edited = false;
}

/**
 * Move item up
 */
void FirefoxProfileRunnerConfig::moveUp() {
    edited = true;
    const int current = m_ui->profiles->currentRow();
    if (current < 1) return;

    // Insert element at index of first visible element above
    int newIdx = current;
    for (int i = current - 1; i >= 0; --i) {
        if (!m_ui->profiles->item(i)->isHidden()) {
            newIdx = i;
            break;
        }
    }
    const auto item = m_ui->profiles->takeItem(current);
    m_ui->profiles->insertItem(newIdx, item);
    m_ui->profiles->setCurrentRow(newIdx);
}

/**
 * Move item down
 */
void FirefoxProfileRunnerConfig::moveDown() {
    edited = true;
    const int current = m_ui->profiles->currentRow();
    const int count = m_ui->profiles->count();
    if (current == -1) return;

    // Insert element at index of first visible element above
    int newIdx = current;
    for (int i = current + 1; i < count; ++i) {
        if (!m_ui->profiles->item(i)->isHidden()) {
            newIdx = i;
            break;
        }
    }
    const auto item = m_ui->profiles->takeItem(current);
    m_ui->profiles->insertItem(newIdx, item);
    m_ui->profiles->setCurrentRow(newIdx);
}

/**
 * Update name of profile
 */
void FirefoxProfileRunnerConfig::applyProfileName() {
    edited = true;
    if (m_ui->profiles->currentRow() == -1)return;
    const QString editedText = m_ui->editProfileName->text();
    const QString textBeforeEditing = m_ui->profiles->currentItem()->text();
    const int count = m_ui->profiles->count();
    m_ui->profiles->currentItem()->setText(editedText);
    for (int i = 0; i < count; ++i) {
        auto *item = m_ui->profiles->item(i);
        if (item->text() == textBeforeEditing) {
            item->setText(editedText);
            break;
        }
    }
    m_ui->editProfileNameApply->setDisabled(true);
    m_ui->editProfileNameCancel->setDisabled(true);
    m_ui->profiles->setFocus();

}

/**
 * Reset text of name edit field to initial name
 */
void FirefoxProfileRunnerConfig::cancelProfileName() {
    if (m_ui->profiles->currentRow() == -1)return;
    m_ui->editProfileName->setText(m_ui->profiles->currentItem()->text());
    m_ui->editProfileName->setFocus();
}

/**
 * Validate buttons to cancel/apply the changes to profile name
 */
void FirefoxProfileRunnerConfig::editProfileName() {
    if (m_ui->profiles->currentRow() == -1) return;
    m_ui->editProfileNameApply->setDisabled(
            m_ui->profiles->currentItem()->text() == m_ui->editProfileName->text() ||
            m_ui->editProfileName->text().isEmpty()
    );
    m_ui->editProfileNameCancel->setDisabled(m_ui->profiles->currentItem()->text() == m_ui->editProfileName->text());

}

/**
 * Hide hide/unhide all private window options
 */
void FirefoxProfileRunnerConfig::showAlwaysPrivateWindows() {
    const int count = m_ui->profiles->count();
    for (int i = 0; i < count; i++) {
        QListWidgetItem *item = m_ui->profiles->item(i);
        if (item->data(32).toList().at(2).toBool()) item->setHidden(!m_ui->showAlwaysPrivateWindows->isChecked());
    }
}

/**
 * Hide default profile from profiles list
 */
void FirefoxProfileRunnerConfig::hideDefaultProfile() {
    const int count = m_ui->profiles->count();
    for (int i = 0; i < count; i++) {
        QListWidgetItem *item = m_ui->profiles->item(i);
        const auto &data = item->data(32).toList();
        if (data.at(1).toBool() && !data.at(2).toBool()) {
            item->setHidden(m_ui->hideDefaultProfile->isChecked());
            break;
        }
    }
}

/**
 *
 */
void FirefoxProfileRunnerConfig::proxychainsSelectionChanged() {
    if (previousProxychainsSelection == "existing") {
        const int itemCount = m_ui->profiles->count();
        for (int i = 0; i < itemCount; ++i) {
            const auto item = m_ui->profiles->item(i);
            item->setData(Qt::CheckStateRole, QVariant());
        }
    } else if (previousProxychainsSelection == "extra") {
        qInfo() << "Extra: TODO Remove extra entries";
    }

    if (m_ui->launchExistingOptionRadioButton->isChecked()) {
        previousProxychainsSelection = "existing";
        const int itemCount = m_ui->profiles->count();
        for (int i = 0; i < itemCount; ++i) {
            const auto item = m_ui->profiles->item(i);
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(Qt::Unchecked);
        }
    } else if (m_ui->showExtraOptionRadioButton->isChecked()) {
        qInfo() << "Extra options TODO: Manual picker ";
        previousProxychainsSelection = "extra";
    } else {
        previousProxychainsSelection = "disabled";
    }
}

/**
 * Hide/Show the general config config widgets
 */
void FirefoxProfileRunnerConfig::toggleGeneralConfigVisibility(const QString &forceHide) {
    bool hide = false;
    if (forceHide == "true") hide = true;
    if (!hide) hide = !m_ui->firefoxGeneralConfigWidget->isHidden();
    if (forceHide == "skip") hide = false;
    m_ui->generalConfigToggleHidePushButton->setIcon(QIcon::fromTheme(hide ? "arrow-down" : "arrow-up"));
    m_ui->firefoxGeneralConfigWidget->setHidden(hide);
}

/**
 * Hide/Show the proxychains config config widgets
 */
void FirefoxProfileRunnerConfig::toggleProxychainsConfigVisibility(const QString &forceHide) {
    bool hide = false;
    if (forceHide == "true") hide = true;
    if (!hide) hide = !m_ui->proxychainsItemsWidget->isHidden();
    if (forceHide == "skip") hide = false;
    m_ui->proxychainsToggleHidePushButton->setIcon(QIcon::fromTheme(hide ? "arrow-down" : "arrow-up"));
    m_ui->proxychainsItemsWidget->setHidden(hide);
}

/**
 * Enable/Disable the launch new instance check box
 */
void FirefoxProfileRunnerConfig::validateProxychainsOptions() {
    m_ui->forceNewInstanceCheckBox->setDisabled(m_ui->disableProxychainsRadioButton->isChecked());
}


#include "firefoxprofilerunner_config.moc"
