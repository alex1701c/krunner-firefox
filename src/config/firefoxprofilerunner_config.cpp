#include "firefoxprofilerunner_config.h"
#include <KSharedConfig>
#include <KPluginFactory>
#include <QtWidgets/QTableWidgetItem>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QMessageBox>
#include <QDebug>
#include <QtCore/QDir>
#include "helper.h"
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
    connect(m_ui->hideDefaultProfile, SIGNAL(clicked(bool)), this, SLOT(changed()));
    connect(m_ui->hideDefaultProfile, SIGNAL(clicked(bool)), this, SLOT(hideDefaultProfile()));
    connect(m_ui->hideDefaultProfile, SIGNAL(clicked(bool)), this, SLOT(itemSelected()));
    connect(m_ui->showAlwaysPrivateWindows, SIGNAL(clicked(bool)), this, SLOT(changed()));
    connect(m_ui->showAlwaysPrivateWindows, SIGNAL(clicked(bool)), this, SLOT(showAlwaysPrivateWindows()));
    connect(m_ui->showAlwaysPrivateWindows, SIGNAL(clicked(bool)), this, SLOT(itemSelected()));
    connect(m_ui->showProxychainsOptionsGloballyCheckBox, SIGNAL(clicked(bool)), this, SLOT(changed()));
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
    connect(m_ui->proxychainsLearnMorePushButton, SIGNAL(clicked(bool)), this, SLOT(learnMoreProxychains()));
    connect(m_ui->proxychainsHideMessagePushButton, SIGNAL(clicked(bool)), this, SLOT(hideMessage()));
    connect(m_ui->disableProxychainsRadioButton, SIGNAL(clicked(bool)), this, SLOT(changed()));
    connect(m_ui->launchExistingOptionRadioButton, SIGNAL(clicked(bool)), this, SLOT(changed()));
    connect(m_ui->showExtraOptionRadioButton, SIGNAL(clicked(bool)), this, SLOT(changed()));
    connect(m_ui->forceNewInstanceCheckBox, SIGNAL(clicked(bool)), this, SLOT(changed()));
    connect(m_ui->proxychainsExtraAddPushButton, SIGNAL(clicked(bool)), this, SLOT(changed()));
    connect(m_ui->proxychainsExtraRemovePushButton, SIGNAL(clicked(bool)), this, SLOT(changed()));
    connect(m_ui->profiles, SIGNAL(itemChanged(QListWidgetItem * )), SLOT(changed()));
    // Proxychains options validation/ui changes
    connect(m_ui->disableProxychainsRadioButton, SIGNAL(clicked(bool)), this, SLOT(validateProxychainsOptions()));
    connect(m_ui->launchExistingOptionRadioButton, SIGNAL(clicked(bool)), this, SLOT(validateProxychainsOptions()));
    connect(m_ui->showExtraOptionRadioButton, SIGNAL(clicked(bool)), this, SLOT(validateProxychainsOptions()));
    connect(m_ui->disableProxychainsRadioButton, SIGNAL(clicked(bool)), this, SLOT(proxychainsSelectionChanged()));
    connect(m_ui->launchExistingOptionRadioButton, SIGNAL(clicked(bool)), this, SLOT(proxychainsSelectionChanged()));
    connect(m_ui->showExtraOptionRadioButton, SIGNAL(clicked(bool)), this, SLOT(proxychainsSelectionChanged()));
    connect(m_ui->proxychainsExtraAddPushButton, SIGNAL(clicked(bool)), this, SLOT(addExtraOption()));
    connect(m_ui->proxychainsExtraRemovePushButton, SIGNAL(clicked(bool)), this, SLOT(removeExtraOption()));
    connect(m_ui->proxychainsExtraComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(validateExtraOptionButtons()));
    connect(m_ui->proxychainsExtraAddPushButton, SIGNAL(clicked(bool)), this, SLOT(validateExtraOptionButtons()));
    connect(m_ui->proxychainsExtraRemovePushButton, SIGNAL(clicked(bool)), this, SLOT(validateExtraOptionButtons()));

    firefoxConfig = KSharedConfig::openConfig(profileManager.firefoxDesktopFile);
    config = KSharedConfig::openConfig("krunnerrc")->group("Runners").group("FirefoxProfileRunner");
    proxychainsInstalled = QFile::exists("/usr/bin/proxychains4");
    firefoxIcon = QIcon::fromTheme(profileManager.launchCommand.endsWith("firefox") ? "firefox" : "firefox-esr");
    firefoxPrivateWindowIcon = QIcon("/usr/share/icons/private_browsing_firefox.svg");
}

void FirefoxProfileRunnerConfig::load() {
    m_ui->registerPrivateWindows->setChecked(stringToBool(config.readEntry("registerPrivateWindows")));
    m_ui->registerNormalWindows->setChecked(stringToBool(config.readEntry("registerNormalWindows", "true")));
    m_ui->hideDefaultProfile->setChecked(stringToBool(config.readEntry("hideDefaultProfile")));
    m_ui->showAlwaysPrivateWindows->setChecked(stringToBool(config.readEntry("showAlwaysPrivateWindows")));

    profiles = profileManager.syncAndGetCustomProfiles(forceProfileSync);
    forceProfileSync = false;
    m_ui->profiles->clear();

    QList<QListWidgetItem *> items;
    QMap<QListWidgetItem *, Profile> itemProfileMap;
    const bool addItemsToSet = config.readEntry("proxychainsIntegration", "disabled") == "existing";
    for (const auto &profile:profiles) {
        // Normal window
        auto *item = new QListWidgetItem();
        item->setText(profile.name);
        QList<QVariant> data = {profile.path, profile.isDefault, "normal", profile.priority};
        item->setData(32, data);
        item->setIcon(firefoxIcon);
        items.append(item);
        // Private window
        auto *item2 = new QListWidgetItem();
        item2->setText(profile.name);
        QList<QVariant> data2 = {profile.path, profile.isDefault, "private", profile.privateWindowPriority};
        item2->setData(32, data2);
        item2->setIcon(firefoxPrivateWindowIcon);
        items.append(item2);
        if (addItemsToSet) {
            itemProfileMap.insert(item, profile);
            itemProfileMap.insert(item2, profile);
        }
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
        m_ui->proxychainsExtraControlsWidget->hide();
        m_ui->forceNewInstanceCheckBox->setChecked(stringToBool(config.readEntry("proxychainsForceNewInstance")));
        m_ui->showProxychainsOptionsGloballyCheckBox->setChecked(stringToBool(config.readEntry("showProxychainsOptionsGlobally")));
        const QString proxychainsChoice = config.readEntry("proxychainsIntegration", "disabled");
        previousProxychainsSelection = proxychainsChoice;
        if (proxychainsChoice == "disabled") m_ui->disableProxychainsRadioButton->setChecked(true);
        else if (proxychainsChoice == "existing") m_ui->launchExistingOptionRadioButton->setChecked(true);
        else m_ui->showExtraOptionRadioButton->setChecked(true);
        // Validate
        m_ui->proxychainsNotInstalledWidget->hide();
        toggleProxychainsConfigVisibility(stringToBool(config.readEntry("proxychainsMinimized")) ? "true" : "skip");
        validateProxychainsOptions();
        loadInitialSettings(itemProfileMap);
    } else {
        m_ui->showProxychainsOptionsGloballyCheckBox->hide();
        m_ui->proxychainsLearnMoreLabel->hide();
        m_ui->proxychainsConfigGroupBox->hide();
        if (config.readEntry("hide_not_installed_message") == "true") m_ui->proxychainsNotInstalledWidget->hide();
    }

    emit changed(false);
}

void FirefoxProfileRunnerConfig::save() {
    // Write general settings
    config.writeEntry("generalMinimized", m_ui->firefoxGeneralConfigWidget->isHidden());
    config.writeEntry("registerNormalWindows", m_ui->registerNormalWindows->isChecked());
    config.writeEntry("registerPrivateWindows", m_ui->registerPrivateWindows->isChecked());
    config.writeEntry("hideDefaultProfile", m_ui->hideDefaultProfile->isChecked());
    config.writeEntry("showAlwaysPrivateWindows", m_ui->showAlwaysPrivateWindows->isChecked());

    // Write proxychains settings
    const bool showExtraProxychainsOptionsGlobally =
            m_ui->showProxychainsOptionsGloballyCheckBox->isChecked() && m_ui->showExtraOptionRadioButton->isChecked();
    if (proxychainsInstalled) {
        config.writeEntry("proxychainsMinimized", m_ui->proxychainsItemsWidget->isHidden());
        config.writeEntry("proxychainsMinimized", m_ui->proxychainsItemsWidget->isHidden());
        config.writeEntry("showProxychainsOptionsGlobally", showExtraProxychainsOptionsGlobally);
        QString proxychainsChoice;
        if (m_ui->disableProxychainsRadioButton->isChecked()) proxychainsChoice = "disabled";
        else if (m_ui->launchExistingOptionRadioButton->isChecked()) proxychainsChoice = "existing";
        else proxychainsChoice = "extra";
        config.writeEntry("proxychainsIntegration", proxychainsChoice);
    }

    // Organize entries in a map
    const int profilesCount = m_ui->profiles->count();
    //  <profilePath, <type, item> >
    QMap<QString, QMap<QString, QListWidgetItem *>> organizedItems;
    for (int i = 0; i < profilesCount; ++i) {
        auto *item = m_ui->profiles->item(i);
        auto data = item->data(32).toList();
        data.replace(3, 100 - i);
        item->setData(32, data);
        auto newData = organizedItems.value(data.at(0).toString());
        newData.insert(data.at(2).toString(), item);
        organizedItems.insert(data.at(0).toString(), newData);
    }

    // Sync profiles with entries from map
    const QString instanceOpt = m_ui->forceNewInstanceCheckBox->isChecked() ? " --new-instance" : "";
    for (auto &profile:profiles) {
        // Reset optional values to false/0
        profile.privateWindowPriority = 0;
        profile.launchNormalWindowWithProxychains = false;
        profile.launchPrivateWindowWithProxychains = false;
        profile.extraNormalWindowProxychainsLaunchOption = false;
        profile.extraNormalWindowProxychainsOptionPriority = 0;
        profile.extraPrivateWindowProxychainsLaunchOption = false;
        profile.extraPrivateWindowProxychainsOptionPriority = 0;

        auto itemMap = organizedItems.value(profile.path);
        for (const auto &key:itemMap.keys()) {
            const auto item = itemMap.value(key);
            const auto itemData = item->data(32).toStringList();
            if (key == "normal") {
                profile.name = item->text();
                profile.priority = itemData.at(3).toInt();
                profile.launchNormalWindowWithProxychains = item->checkState() == Qt::Checked;
            } else if (key == "private") {
                profile.privateWindowPriority = itemData.at(3).toInt();
                profile.launchPrivateWindowWithProxychains = item->checkState() == Qt::Checked;
            } else if (key == "proxychains-normal") {
                profile.extraNormalWindowProxychainsLaunchOption = true;
                profile.extraNormalWindowProxychainsOptionPriority = itemData.at(3).toInt();
            } else if (key == "proxychains-private") {
                profile.extraPrivateWindowProxychainsLaunchOption = true;
                profile.extraPrivateWindowProxychainsOptionPriority = itemData.at(3).toInt();
            }
        }
        profile.writeConfigChanges(firefoxConfig, instanceOpt);
    }

    // Change registering based on new settings
    profileManager.changeProfileRegistering(m_ui->registerNormalWindows->isChecked(), m_ui->registerPrivateWindows->isChecked(),
                                            showExtraProxychainsOptionsGlobally, firefoxConfig);
    // New runner instance has latest configuration
    system("kquitapp5 krunner;kstart5 krunner > /dev/null 2&>1");

    emit changed(true);
}

void FirefoxProfileRunnerConfig::defaults() {
    m_ui->registerNormalWindows->setChecked(true);
    m_ui->registerPrivateWindows->setChecked(false);
    m_ui->hideDefaultProfile->setChecked(false);
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
    m_ui->editProfileName->setText(m_ui->profiles->currentItem()->text().remove("Proxychains: "));
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
    const QString editedText = m_ui->editProfileName->text();
    if (m_ui->profiles->currentRow() == -1) return;
    const QString textBeforeEditing = m_ui->profiles->currentItem()->text();
    const int count = m_ui->profiles->count();

    if (!proxychainsInstalled) {
        m_ui->profiles->currentItem()->setText(editedText);
        for (int i = 0; i < count; ++i) {
            auto *item = m_ui->profiles->item(i);
            if (item->text() == textBeforeEditing) {
                item->setText(editedText);
                break;
            }
        }
    } else {
        const QString rawTextBeforeEditing = QString(textBeforeEditing).remove("Proxychains: ");
        for (int i = 0; i < count; ++i) {
            auto *item = m_ui->profiles->item(i);
            const QString itemText = item->text();
            if (itemText == rawTextBeforeEditing) item->setText(editedText);
            else if (itemText == textBeforeEditing) item->setText("Proxychains: " + editedText);
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
    if (m_ui->profiles->currentRow() == -1) return;
    m_ui->editProfileName->setText(m_ui->profiles->currentItem()->text());
    m_ui->editProfileName->setFocus();
}

/**
 * Validate buttons to cancel/apply the changes to profile name
 */
void FirefoxProfileRunnerConfig::editProfileName() {
    if (m_ui->profiles->currentRow() == -1) return;
    m_ui->editProfileNameApply->setDisabled(
            m_ui->profiles->currentItem()->text() == m_ui->editProfileName->text() || m_ui->editProfileName->text().isEmpty());
    m_ui->editProfileNameCancel->setDisabled(m_ui->profiles->currentItem()->text() == m_ui->editProfileName->text());

}

/**
 * Hide hide/unhide all private window options
 */
void FirefoxProfileRunnerConfig::showAlwaysPrivateWindows() {
    const int count = m_ui->profiles->count();
    for (int i = 0; i < count; i++) {
        QListWidgetItem *item = m_ui->profiles->item(i);
        if (item->data(32).toList().at(2).toString() == "private") {
            item->setHidden(!m_ui->showAlwaysPrivateWindows->isChecked());
        }
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
        if (data.at(1).toBool() && data.at(2).toString() == "normal") {
            item->setHidden(m_ui->hideDefaultProfile->isChecked());
            break;
        }
    }
}

/**
 * Load intitial settings, uses the previousProxychainsSelection variable to determine which option should
 * be loaded
 * @param itemProfileMap Used to get the profile based on the QListWidgetItem, this profile
 * is used to determine the CheckedState of the item.
 * This parameter is only required if the data for the "Use existing profiles" option should be loaded
 */
void FirefoxProfileRunnerConfig::loadInitialSettings(const QMap<QListWidgetItem *, Profile> &itemProfileMap) {
    const int itemCount = m_ui->profiles->count();
    if (previousProxychainsSelection == "existing") {
        for (int i = 0; i < itemCount; ++i) {
            const auto item = m_ui->profiles->item(i);
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            const bool isNormalWindow = item->data(32).toStringList().at(2) == "normal";
            const auto profile = itemProfileMap.value(item);

            const auto state = ((isNormalWindow && profile.launchNormalWindowWithProxychains) ||
                                (!isNormalWindow && profile.launchPrivateWindowWithProxychains)) ? Qt::Checked : Qt::Unchecked;
            item->setCheckState(state);
        }
    } else if (previousProxychainsSelection == "extra") {
        m_ui->proxychainsExtraControlsWidget->setHidden(false);
        QList<QListWidgetItem *> additionalItems;
        for (const auto &profile:profiles) {
            if (profile.extraNormalWindowProxychainsLaunchOption) {
                auto *item = new QListWidgetItem();
                item->setText("Proxychains: " + profile.name);
                QList<QVariant> data = {profile.path, false, "proxychains-normal", profile.extraNormalWindowProxychainsOptionPriority};
                item->setData(32, data);
                item->setIcon(firefoxIcon);
                additionalItems.append(item);
            }
            if (profile.extraPrivateWindowProxychainsLaunchOption) {
                auto *item = new QListWidgetItem();
                item->setText("Proxychains: " + profile.name);
                QList<QVariant> data = {profile.path, false, "proxychains-private",
                                        profile.extraPrivateWindowProxychainsOptionPriority};
                item->setData(32, data);
                item->setIcon(firefoxPrivateWindowIcon);
                additionalItems.append(item);
            }
            m_ui->proxychainsExtraComboBox->addItem(firefoxIcon, profile.name, QStringList(
                    {profile.path, "proxychains-normal", boolToString(profile.extraNormalWindowProxychainsLaunchOption)}));
            m_ui->proxychainsExtraComboBox->addItem(firefoxPrivateWindowIcon, profile.name, QStringList(
                    {profile.path, "proxychains-private", boolToString(profile.extraPrivateWindowProxychainsLaunchOption)}));
        }
        for (int i = itemCount - 1; i > 0; --i) {
            additionalItems.append(m_ui->profiles->takeItem(i));
        }
        std::sort(additionalItems.begin(), additionalItems.end(), [](QListWidgetItem *item1, QListWidgetItem *item2) -> bool {
            return item1->data(32).toList().last() > item2->data(32).toList().last();
        });
        for (const auto &item:additionalItems) m_ui->profiles->addItem(item);
    }
    m_ui->showProxychainsOptionsGloballyCheckBox->setHidden(previousProxychainsSelection != "extra");
}

/**
 * Remove old config for proxychains and enable new one
 * For the "Launch existing ..." option the checkboxes next to the profiles are addded/removed
 * For the "Show extra .." option the proxychainsExtraControlsWidget gets shown/hidden and if the option gets
 * unchecked the extra entries are removed from the ListView
 */
void FirefoxProfileRunnerConfig::proxychainsSelectionChanged() {
    // Return if the current checkbox has been clicked
    if ((previousProxychainsSelection == "existing" && m_ui->launchExistingOptionRadioButton->isChecked()) ||
        (previousProxychainsSelection == "extra" && m_ui->showExtraOptionRadioButton->isChecked()) ||
        (previousProxychainsSelection == "disabled" && m_ui->disableProxychainsRadioButton->isChecked())) {
        return;
    }
    // Remove old config
    if (previousProxychainsSelection == "existing") {
        const int itemCount = m_ui->profiles->count();
        for (int i = 0; i < itemCount; ++i) {
            const auto item = m_ui->profiles->item(i);
            item->setData(Qt::CheckStateRole, QVariant());
        }
    } else if (previousProxychainsSelection == "extra") {
        const int itemCount = m_ui->profiles->count();
        QList<int> toRemove;
        for (int i = 0; i < itemCount; ++i) {
            const auto item = m_ui->profiles->item(i);
            if (item->text().startsWith("Proxychains: ")) toRemove.append(i);
        }
        const int rmCount = toRemove.count();
        for (int i = 0; i < rmCount; ++i) {
            m_ui->profiles->model()->removeRow(toRemove.at(i) - i);
        }
        m_ui->proxychainsExtraControlsWidget->hide();
    }

    // Enable new config
    if (m_ui->launchExistingOptionRadioButton->isChecked()) {
        previousProxychainsSelection = "existing";
        const int itemCount = m_ui->profiles->count();
        for (int i = 0; i < itemCount; ++i) {
            const auto item = m_ui->profiles->item(i);
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(Qt::Unchecked);
        }
    } else if (m_ui->showExtraOptionRadioButton->isChecked()) {
        m_ui->proxychainsExtraControlsWidget->setHidden(false);
        previousProxychainsSelection = "extra";
        m_ui->proxychainsExtraComboBox->clear();
        for (const auto &profile:profiles) {
            m_ui->proxychainsExtraComboBox->addItem(firefoxIcon, profile.name,
                                                    QStringList({profile.path, "proxychains-normal", "false"}));
            m_ui->proxychainsExtraComboBox->addItem(firefoxPrivateWindowIcon, profile.name,
                                                    QStringList({profile.path, "proxychains-private", "false"}));
        }
    } else {
        previousProxychainsSelection = "disabled";
    }
    m_ui->showProxychainsOptionsGloballyCheckBox->setHidden(previousProxychainsSelection != "extra");
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

/**
 * Disable/enable the add/remove buttons for the proxychains combobox
 */
void FirefoxProfileRunnerConfig::validateExtraOptionButtons() {
    const int currentIndex = m_ui->proxychainsExtraComboBox->currentIndex();
    if (currentIndex == -1) return;
    const bool alreadyExists = m_ui->proxychainsExtraComboBox->itemData(currentIndex).toList().at(2).toBool();
    m_ui->proxychainsExtraAddPushButton->setDisabled(alreadyExists);
    m_ui->proxychainsExtraRemovePushButton->setDisabled(!alreadyExists);
}

/**
 * Add selected proxychains option to the list of profiles and mark it as added in combo box
 */
void FirefoxProfileRunnerConfig::addExtraOption() {
    Profile profile{};
    const int currentIndex = m_ui->proxychainsExtraComboBox->currentIndex();
    auto profileInfo = m_ui->proxychainsExtraComboBox->itemData(currentIndex).toStringList();
    const auto profilePath = profileInfo.at(0);
    for (const auto &p:profiles) {
        if (p.path == profilePath) {
            profile = p;
            break;
        }
    }
    // Create item and add it to profiles list
    auto *item = new QListWidgetItem();
    item->setText("Proxychains: " + profile.name);
    QList<QVariant> data = {profile.path, profile.isDefault, profileInfo.at(1), profile.priority};
    item->setData(32, data);
    item->setIcon(profileInfo.at(1) == "proxychains-normal" ? firefoxIcon : firefoxPrivateWindowIcon);
    m_ui->profiles->addItem(item);
    profileInfo.replace(2, "true");
    m_ui->proxychainsExtraComboBox->setItemData(currentIndex, profileInfo);
}

/**
 * Remove selected proxychains option from the list of profiles and mark it as removed in combo box
 */
void FirefoxProfileRunnerConfig::removeExtraOption() {
    const int currentIndex = m_ui->proxychainsExtraComboBox->currentIndex();
    auto profileInfo = m_ui->proxychainsExtraComboBox->itemData(currentIndex).toStringList();
    const QString profilePath = profileInfo.at(0);
    const QString profileType = profileInfo.at(1);
    const int profileItemCount = m_ui->profiles->count();

    // Find and remove item based on path and type
    for (int i = 0; i < profileItemCount; ++i) {
        const auto item = m_ui->profiles->item(i);
        const auto itemData = item->data(32).toStringList();
        if (itemData.at(0) == profilePath && itemData.at(2) == profileType) {
            m_ui->profiles->model()->removeRow(i);
            profileInfo.replace(2, "false");
            m_ui->proxychainsExtraComboBox->setItemData(currentIndex, profileInfo);
            break;
        }
    }
}

void FirefoxProfileRunnerConfig::learnMoreProxychains() {
    m_ui->proxychainsLearnMoreLabel->setHidden(!m_ui->proxychainsLearnMoreLabel->isHidden());
}

void FirefoxProfileRunnerConfig::hideMessage() {
    config.writeEntry("hide_not_installed_message", true);
    m_ui->proxychainsNotInstalledWidget->setHidden(true);
}


#include "firefoxprofilerunner_config.moc"
