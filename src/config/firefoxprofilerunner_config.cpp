#include "firefoxprofilerunner_config.h"
#include <KSharedConfig>
#include <KPluginFactory>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QMessageBox>
#include <QDebug>
#include <QtCore/QDir>
#include <QtCore/QProcess>
#include <Config.h>

K_PLUGIN_FACTORY(FirefoxProfileRunnerConfigFactory,
                 registerPlugin<FirefoxRunnerConfig>("kcm_krunner_firefoxprofilerunner");)

FirefoxProfileRunnerConfigForm::FirefoxProfileRunnerConfigForm(QWidget *parent) : QWidget(parent) {
    setupUi(this);
}

FirefoxRunnerConfig::FirefoxRunnerConfig(QWidget *parent, const QVariantList &args)
        : KCModule(parent, args) {
    m_ui = new FirefoxProfileRunnerConfigForm(this);
    auto *layout = new QGridLayout(this);
    layout->addWidget(m_ui, 0, 0);

    proxychainsInstalled = QFile::exists("/usr/bin/proxychains4");
    connectSignals();
    if (proxychainsInstalled) {
        connectOptionalProxySignals();
    }

    firefoxConfig = KSharedConfig::openConfig(profileManager.firefoxDesktopFile);
    config = KSharedConfig::openConfig(Config::ConfigFile)->group(Config::MainGroup);
    config.config()->reparseConfiguration();
    firefoxIcon = QIcon::fromTheme(profileManager.launchCommand.endsWith("firefox") ? "firefox" : "firefox-esr");
    firefoxPrivateWindowIcon = QIcon::fromTheme("private_browsing_firefox");
}

void FirefoxRunnerConfig::load() {
    m_ui->registerPrivateWindows->setChecked(config.readEntry(Config::RegisterPrivateWindows, true));
    m_ui->registerNormalWindows->setChecked(config.readEntry(Config::RegisterNormalWindows, true));
    m_ui->hideDefaultProfile->setChecked(config.readEntry(Config::HideDefaultProfile, false));
    m_ui->showAlwaysPrivateWindows->setChecked(config.readEntry(Config::ShowAlwaysPrivateWindows, true));
    m_ui->privateWindowAction->setChecked(config.readEntry(Config::PrivateWindowAction, false));
    privateWindowsAsActionsChanged();

    profileManager = ProfileManager();
    profiles = profileManager.syncAndGetCustomProfiles(forceProfileSync);
    forceProfileSync = false;
    m_ui->profiles->clear();

    QList<QListWidgetItem *> items;
    QMap<QListWidgetItem *, Profile> itemProfileMap;
    const bool addItemsToSet = config.readEntry(Config::ProxychainsIntegration, "disabled") == "existing";
    for (const auto &profile: qAsConst(profiles)) {
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
    std::sort(items.begin(), items.end(), [](const QListWidgetItem *item1, const QListWidgetItem *item2) -> bool {
        return item1->data(32).toList().last() > item2->data(32).toList().last();
    });
    for (const auto &item: qAsConst(items)) {
        m_ui->profiles->addItem(item);
    }

    // Initial validation
    showAlwaysPrivateWindows();
    if (config.readEntry(Config::GeneralMinimized, false)) {
        toggleGeneralConfigVisibility();
    }

    if (proxychainsInstalled) {
        // Load data
        m_ui->proxychainsExtraControlsWidget->hide();
        m_ui->forceNewInstanceCheckBox->setChecked(config.readEntry(Config::ProxychainsForceNewInstance, false));
        m_ui->showProxychainsOptionsGloballyCheckBox->setChecked(
                config.readEntry(Config::ShowProxychainsOptionsGlobally, false));
        const QString proxychainsChoice = config.readEntry(Config::ProxychainsIntegration, "disabled");
        previousProxychainsSelection = proxychainsChoice;
        if (proxychainsChoice == "disabled") m_ui->disableProxychainsRadioButton->setChecked(true);
        else if (proxychainsChoice == "existing") m_ui->launchExistingOptionRadioButton->setChecked(true);
        else m_ui->showExtraOptionRadioButton->setChecked(true);
        // Validate
        m_ui->proxychainsNotInstalledWidget->hide();
        if (config.readEntry(Config::ProxychainsMinimized, false)) {
            toggleProxychainsConfigVisibility();
        }
        validateProxychainsOptions();
        loadInitialProxySettings(itemProfileMap);
    } else {
        m_ui->showProxychainsOptionsGloballyCheckBox->hide();
        m_ui->proxychainsLearnMoreLabel->hide();
        m_ui->proxychainsConfigGroupBox->hide();
        m_ui->proxychainsNotInstalledWidget->setHidden(config.readEntry(Config::HideProxychainsMsg, false));
    }
    hideDefaultProfile();
}

void FirefoxRunnerConfig::save() {
    // Write general settings
    config.writeEntry(Config::GeneralMinimized, m_ui->firefoxGeneralConfigWidget->isHidden());
    config.writeEntry(Config::RegisterNormalWindows, m_ui->registerNormalWindows->isChecked());
    config.writeEntry(Config::RegisterPrivateWindows, m_ui->registerPrivateWindows->isChecked());
    config.writeEntry(Config::PrivateWindowAction, m_ui->privateWindowAction->isChecked());
    config.writeEntry(Config::HideDefaultProfile,
                      m_ui->hideDefaultProfile->isEnabled() && m_ui->hideDefaultProfile->isChecked());
    config.writeEntry(Config::ShowAlwaysPrivateWindows, m_ui->showAlwaysPrivateWindows->isEnabled()
                                                        && m_ui->showAlwaysPrivateWindows->isChecked());

    // Write proxychains settings
    const bool showExtraProxychainsOptionsGlobally =
            m_ui->showProxychainsOptionsGloballyCheckBox->isChecked() && m_ui->showExtraOptionRadioButton->isChecked();
    if (proxychainsInstalled) {
        config.writeEntry(Config::ProxychainsMinimized, m_ui->proxychainsItemsWidget->isHidden());
        config.writeEntry(Config::ProxychainsForceNewInstance, m_ui->forceNewInstanceCheckBox->isChecked());
        config.writeEntry(Config::ShowProxychainsOptionsGlobally, showExtraProxychainsOptionsGlobally);
        QString proxychainsChoice;
        if (m_ui->disableProxychainsRadioButton->isChecked()) proxychainsChoice = "disabled";
        else if (m_ui->launchExistingOptionRadioButton->isChecked()) proxychainsChoice = "existing";
        else proxychainsChoice = "extra";
        config.writeEntry(Config::ProxychainsIntegration, proxychainsChoice);
    }

    // Organize entries in a map <profilePath, <type, item> >
    QMap<QString, QMap<QString, QListWidgetItem *>> organizedItems;
    for (int i = 0; i < m_ui->profiles->count(); ++i) {
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
    profileManager.changeProfileRegistering(m_ui->registerNormalWindows->isChecked(),
                                            m_ui->registerPrivateWindows->isChecked(),
                                            showExtraProxychainsOptionsGlobally, firefoxConfig);
    // New runner instance has latest configuration
    QProcess::startDetached("bash", QStringList() << "-c" << "kquitapp5 krunner;kstart5 krunner > /dev/null 2&>1");
}

void FirefoxRunnerConfig::defaults() {
    m_ui->registerNormalWindows->setChecked(true);
    m_ui->registerPrivateWindows->setChecked(true);
    m_ui->hideDefaultProfile->setChecked(false);
    m_ui->showAlwaysPrivateWindows->setChecked(true);
    m_ui->privateWindowAction->setChecked(false);
    privateWindowsAsActionsChanged();

    if (proxychainsInstalled) {
        m_ui->disableProxychainsRadioButton->setChecked(true);
        m_ui->forceNewInstanceCheckBox->setChecked(false);
        m_ui->showProxychainsOptionsGloballyCheckBox->setChecked(false);
        proxychainsSelectionChanged();
    }
    hideDefaultProfile();

#if KCMUTILS_VERSION >= QT_VERSION_CHECK(5, 64, 0)
    emit markAsChanged();
#else
    emit changed();
#endif
}

void FirefoxRunnerConfig::connectSignals() {
#if KCMUTILS_VERSION >= QT_VERSION_CHECK(5, 64, 0)
    const auto changedSlotPointer = &FirefoxRunnerConfig::markAsChanged;
#else
    const auto changedSlotPointer = static_cast<void (FirefoxRunnerConfig::*)()>(&FirefoxRunnerConfig::changed);
#endif
    const auto toggleConfigPointer = static_cast<void (FirefoxRunnerConfig::*)()>(&FirefoxRunnerConfig::toggleGeneralConfigVisibility);

    // General settings
    connect(m_ui->registerNormalWindows, &QCheckBox::clicked, this, changedSlotPointer);
    connect(m_ui->registerPrivateWindows, &QCheckBox::clicked, this, changedSlotPointer);
    connect(m_ui->hideDefaultProfile, &QCheckBox::clicked, this, changedSlotPointer);
    connect(m_ui->hideDefaultProfile, &QCheckBox::clicked, this, &FirefoxRunnerConfig::hideDefaultProfile);
    connect(m_ui->hideDefaultProfile, &QCheckBox::clicked, this, &FirefoxRunnerConfig::itemSelected);
    connect(m_ui->showAlwaysPrivateWindows, &QCheckBox::clicked, this, changedSlotPointer);
    connect(m_ui->showAlwaysPrivateWindows, &QCheckBox::clicked, this, &FirefoxRunnerConfig::showAlwaysPrivateWindows);
    connect(m_ui->showAlwaysPrivateWindows, &QCheckBox::clicked, this, &FirefoxRunnerConfig::itemSelected);
    connect(m_ui->showProxychainsOptionsGloballyCheckBox, &QCheckBox::clicked, this, changedSlotPointer);
    connect(m_ui->privateWindowAction, &QCheckBox::clicked, this, changedSlotPointer);
    connect(m_ui->privateWindowAction, &QCheckBox::clicked, this, &FirefoxRunnerConfig::privateWindowsAsActionsChanged);

    // Different item gets selected
    connect(m_ui->profiles, &QListWidget::itemSelectionChanged, this, &FirefoxRunnerConfig::itemSelected);
    connect(m_ui->profiles, &QListWidget::itemSelectionChanged, this, &FirefoxRunnerConfig::editProfileName);
    // Order gets changes
    connect(m_ui->moveUp, &QPushButton::clicked, this, &FirefoxRunnerConfig::moveUp);
    connect(m_ui->moveDown, &QPushButton::clicked, this, &FirefoxRunnerConfig::moveDown);
    connect(m_ui->moveUp, &QPushButton::clicked, this, changedSlotPointer);
    connect(m_ui->moveDown, &QPushButton::clicked, this, changedSlotPointer);
    // Edit profile name
    connect(m_ui->editProfileName, &QLineEdit::textChanged, this, &FirefoxRunnerConfig::editProfileName);
    connect(m_ui->editProfileNameApply, &QPushButton::clicked, this, &FirefoxRunnerConfig::applyProfileName);
    connect(m_ui->editProfileNameApply, &QPushButton::clicked, this, changedSlotPointer);
    connect(m_ui->editProfileNameCancel, &QPushButton::clicked, this, &FirefoxRunnerConfig::cancelProfileName);
    // Refresh profiles button
    connect(m_ui->refreshProfiles, &QPushButton::clicked, this, &FirefoxRunnerConfig::refreshProfiles);
    connect(m_ui->refreshProfiles, &QPushButton::clicked, this, changedSlotPointer);
    // Hide/unhide buttons
    connect(m_ui->generalConfigToggleHidePushButton, &QPushButton::clicked, this, toggleConfigPointer);
    connect(m_ui->generalConfigToggleHidePushButton, &QPushButton::clicked, this, changedSlotPointer);
    connect(m_ui->proxychainsToggleHidePushButton, &QPushButton::clicked,
            this, &FirefoxRunnerConfig::toggleProxychainsConfigVisibility);
    connect(m_ui->proxychainsToggleHidePushButton, &QPushButton::clicked, this, changedSlotPointer);
    // These signals are needed even if proxychains is not installed
    connect(m_ui->proxychainsLearnMorePushButton, &QPushButton::clicked,
            this, &FirefoxRunnerConfig::learnMoreProxychains);
    connect(m_ui->proxychainsHideMessagePushButton, &QPushButton::clicked,
            this, &FirefoxRunnerConfig::hideMessage);
}

void FirefoxRunnerConfig::connectOptionalProxySignals() {
#if KCMUTILS_VERSION >= QT_VERSION_CHECK(5, 64, 0)
    const auto changedSlotPointer = &FirefoxRunnerConfig::markAsChanged;
#else
    const auto changedSlotPointer = static_cast<void (FirefoxRunnerConfig::*)()>(&FirefoxRunnerConfig::changed);
#endif

    // Proxychains options changed
    connect(m_ui->disableProxychainsRadioButton, &QRadioButton::clicked, this, changedSlotPointer);
    connect(m_ui->launchExistingOptionRadioButton, &QRadioButton::clicked, this, changedSlotPointer);
    connect(m_ui->showExtraOptionRadioButton, &QRadioButton::clicked, this, changedSlotPointer);
    connect(m_ui->forceNewInstanceCheckBox, &QCheckBox::clicked, this, changedSlotPointer);
    connect(m_ui->proxychainsExtraAddPushButton, &QCheckBox::clicked, this, changedSlotPointer);
    connect(m_ui->proxychainsExtraRemovePushButton, &QCheckBox::clicked, this, changedSlotPointer);
    connect(m_ui->profiles, &QListWidget::itemChanged, this, changedSlotPointer);
    // Proxychains options validation/ui changes
    connect(m_ui->disableProxychainsRadioButton, &QRadioButton::clicked,
            this, &FirefoxRunnerConfig::validateProxychainsOptions);
    connect(m_ui->launchExistingOptionRadioButton, &QRadioButton::clicked,
            this, &FirefoxRunnerConfig::validateProxychainsOptions);
    connect(m_ui->showExtraOptionRadioButton, &QRadioButton::clicked,
            this, &FirefoxRunnerConfig::validateProxychainsOptions);
    connect(m_ui->disableProxychainsRadioButton, &QRadioButton::clicked,
            this, &FirefoxRunnerConfig::proxychainsSelectionChanged);
    connect(m_ui->launchExistingOptionRadioButton, &QRadioButton::clicked,
            this, &FirefoxRunnerConfig::proxychainsSelectionChanged);
    connect(m_ui->showExtraOptionRadioButton, &QRadioButton::clicked,
            this, &FirefoxRunnerConfig::proxychainsSelectionChanged);
    connect(m_ui->proxychainsExtraAddPushButton, &QPushButton::clicked,
            this, &FirefoxRunnerConfig::addExtraOption);
    connect(m_ui->proxychainsExtraRemovePushButton, &QPushButton::clicked,
            this, &FirefoxRunnerConfig::removeExtraOption);
    connect(m_ui->proxychainsExtraComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &FirefoxRunnerConfig::validateExtraOptionButtons);
    connect(m_ui->proxychainsExtraAddPushButton, &QPushButton::clicked, this,
            &FirefoxRunnerConfig::validateExtraOptionButtons);
    connect(m_ui->proxychainsExtraRemovePushButton, &QPushButton::clicked, this,
            &FirefoxRunnerConfig::validateExtraOptionButtons);
}

/**
 * Enable/Disable Up/Down buttons
 */
void FirefoxRunnerConfig::itemSelected() {
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
void FirefoxRunnerConfig::refreshProfiles() {
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
    hideDefaultProfile();
}

void FirefoxRunnerConfig::privateWindowsAsActionsChanged() {
    m_ui->hideDefaultProfile->setDisabled(m_ui->privateWindowAction->isChecked());
    m_ui->showAlwaysPrivateWindows->setDisabled(m_ui->privateWindowAction->isChecked());
}

/**
 * Move item up
 */
void FirefoxRunnerConfig::moveUp() {
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
void FirefoxRunnerConfig::moveDown() {
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
void FirefoxRunnerConfig::applyProfileName() {
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
        // Rename entries in sortable list
        const QString rawTextBeforeEditing = QString(textBeforeEditing).remove("Proxychains: ");
        for (int i = 0; i < count; ++i) {
            auto *item = m_ui->profiles->item(i);
            const QString itemText = item->text();
            if (itemText == rawTextBeforeEditing) item->setText(editedText);
            else if (itemText == textBeforeEditing) item->setText("Proxychains: " + editedText);
        }

        // Rename entries in combo box
        for (int i = 0; i < m_ui->proxychainsExtraComboBox->count(); ++i) {
            const QString itemText = m_ui->proxychainsExtraComboBox->itemText(i);
            if (itemText == rawTextBeforeEditing) m_ui->proxychainsExtraComboBox->setItemText(i, editedText);
        }
    }
    m_ui->editProfileNameApply->setDisabled(true);
    m_ui->editProfileNameCancel->setDisabled(true);
    m_ui->profiles->setFocus();
}

/**
 * Reset text of name edit field to initial name
 */
void FirefoxRunnerConfig::cancelProfileName() {
    if (m_ui->profiles->currentRow() != -1) {
        m_ui->editProfileName->setText(m_ui->profiles->currentItem()->text());
        m_ui->editProfileName->setFocus();
    }
}

/**
 * Validate buttons to cancel/apply the changes to profile name
 */
void FirefoxRunnerConfig::editProfileName() {
    if (m_ui->profiles->currentRow() != -1) {
        m_ui->editProfileNameApply->setDisabled(
                m_ui->profiles->currentItem()->text() == m_ui->editProfileName->text() ||
                m_ui->editProfileName->text().isEmpty());
        m_ui->editProfileNameCancel->setDisabled(
                m_ui->profiles->currentItem()->text() == m_ui->editProfileName->text());
    }
}

/**
 * Hide hide/unhide all private window options
 */
void FirefoxRunnerConfig::showAlwaysPrivateWindows() {
    for (int i = 0; i < m_ui->profiles->count(); ++i) {
        QListWidgetItem *item = m_ui->profiles->item(i);
        if (item->data(32).toList().at(2).toString() == "private") {
            item->setHidden(!m_ui->showAlwaysPrivateWindows->isChecked());
        }
    }
}

/**
 * Hide default profile from profiles list if checkbox is checked
 */
void FirefoxRunnerConfig::hideDefaultProfile() {
    const bool hide = m_ui->hideDefaultProfile->isChecked() && m_ui->hideDefaultProfile->isEnabled();
    for (int i = 0; i < m_ui->profiles->count(); ++i) {
        QListWidgetItem *item = m_ui->profiles->item(i);
        const auto &data = item->data(32).toList();
        if (data.at(1).toBool() && data.at(2).toString() == "normal") {
            item->setHidden(hide);
            break;
        }
    }
}

/**
 * Load initial settings, uses the previousProxychainsSelection variable to determine which option should
 * be loaded
 * @param itemProfileMap Used to get the profile based on the QListWidgetItem, this profile
 * is used to determine the CheckedState of the item.
 * This parameter is only required if the data for the "Use existing profiles" option should be loaded
 */
void FirefoxRunnerConfig::loadInitialProxySettings(const QMap<QListWidgetItem *, Profile> &itemProfileMap) {
    if (previousProxychainsSelection == "existing") {
        for (int i = 0; i < m_ui->profiles->count(); ++i) {
            const auto item = m_ui->profiles->item(i);
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            const bool isNormalWindow = item->data(32).toStringList().at(2) == "normal";
            const auto profile = itemProfileMap.value(item);

            const auto state = ((isNormalWindow && profile.launchNormalWindowWithProxychains) ||
                                (!isNormalWindow && profile.launchPrivateWindowWithProxychains)) ? Qt::Checked
                                                                                                 : Qt::Unchecked;
            item->setCheckState(state);
        }
    } else if (previousProxychainsSelection == "extra") {
        m_ui->proxychainsExtraControlsWidget->setHidden(false);
        m_ui->proxychainsExtraComboBox->clear();
        QList<QListWidgetItem *> additionalItems;
        for (const auto &profile: qAsConst(profiles)) {
            if (profile.extraNormalWindowProxychainsLaunchOption) {
                auto *item = new QListWidgetItem();
                item->setText("Proxychains: " + profile.name);
                QList<QVariant> data = {profile.path, false, "proxychains-normal",
                                        profile.extraNormalWindowProxychainsOptionPriority};
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
                    {profile.path, "proxychains-normal",
                     QVariant(profile.extraNormalWindowProxychainsLaunchOption).toString()}));
            m_ui->proxychainsExtraComboBox->addItem(firefoxPrivateWindowIcon, profile.name, QStringList(
                    {profile.path, "proxychains-private",
                     QVariant(profile.extraPrivateWindowProxychainsLaunchOption).toString()}));
        }
        for (int i = m_ui->profiles->count()- 1; i > 0; --i) {
            additionalItems.append(m_ui->profiles->takeItem(i));
        }
        std::sort(additionalItems.begin(), additionalItems.end(),
                  [](QListWidgetItem *item1, QListWidgetItem *item2) -> bool {
                      return item1->data(32).toList().last() > item2->data(32).toList().last();
                  });
        for (const auto &item: additionalItems) {
            m_ui->profiles->addItem(item);
        }
    }
    m_ui->showProxychainsOptionsGloballyCheckBox->setHidden(previousProxychainsSelection != "extra");
}

/**
 * Remove old config for proxychains and enable new one
 * For the "Launch existing ..." option the checkboxes next to the profiles are addded/removed
 * For the "Show extra .." option the proxychainsExtraControlsWidget gets shown/hidden and if the option gets
 * unchecked the extra entries are removed from the ListView
 */
void FirefoxRunnerConfig::proxychainsSelectionChanged() {
    // Return if the current checkbox has been clicked
    if ((previousProxychainsSelection == "existing" && m_ui->launchExistingOptionRadioButton->isChecked()) ||
        (previousProxychainsSelection == "extra" && m_ui->showExtraOptionRadioButton->isChecked()) ||
        (previousProxychainsSelection == "disabled" && m_ui->disableProxychainsRadioButton->isChecked())) {
        return;
    }
    // Remove old config
    if (previousProxychainsSelection == "existing") {
        for (int i = 0; i < m_ui->profiles->count(); ++i) {
            const auto item = m_ui->profiles->item(i);
            item->setData(Qt::CheckStateRole, QVariant());
        }
    } else if (previousProxychainsSelection == "extra") {
        QList<int> toRemove;
        for (int i = 0; i < m_ui->profiles->count(); ++i) {
            const auto item = m_ui->profiles->item(i);
            if (item->text().startsWith("Proxychains: ")) toRemove.append(i);
        }
        for (int i = 0; i < toRemove.count(); ++i) {
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
        for (const auto &profile: qAsConst(profiles)) {
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
void FirefoxRunnerConfig::toggleGeneralConfigVisibility() {
    bool hide = !m_ui->firefoxGeneralConfigWidget->isHidden();
    m_ui->generalConfigToggleHidePushButton->setIcon(QIcon::fromTheme(hide ? "arrow-down" : "arrow-up"));
    m_ui->firefoxGeneralConfigWidget->setHidden(hide);
}

/**
 * Hide/Show the proxychains config config widgets
 */
void FirefoxRunnerConfig::toggleProxychainsConfigVisibility() {
    bool hide = !m_ui->proxychainsItemsWidget->isHidden();
    m_ui->proxychainsToggleHidePushButton->setIcon(QIcon::fromTheme(hide ? "arrow-down" : "arrow-up"));
    m_ui->proxychainsItemsWidget->setHidden(hide);
}

/**
 * Enable/Disable the launch new instance check box
 */
void FirefoxRunnerConfig::validateProxychainsOptions() {
    m_ui->forceNewInstanceCheckBox->setDisabled(m_ui->disableProxychainsRadioButton->isChecked());
}

/**
 * Disable/enable the add/remove buttons for the proxychains combobox
 */
void FirefoxRunnerConfig::validateExtraOptionButtons() {
    const int currentIndex = m_ui->proxychainsExtraComboBox->currentIndex();
    if (currentIndex == -1) return;
    const bool alreadyExists = m_ui->proxychainsExtraComboBox->itemData(currentIndex).toList().at(2).toBool();
    m_ui->proxychainsExtraAddPushButton->setDisabled(alreadyExists);
    m_ui->proxychainsExtraRemovePushButton->setDisabled(!alreadyExists);
}

/**
 * Add selected proxychains option to the list of profiles and mark it as added in combo box
 */
void FirefoxRunnerConfig::addExtraOption() {
    Profile profile{};
    const int currentIndex = m_ui->proxychainsExtraComboBox->currentIndex();
    auto profileInfo = m_ui->proxychainsExtraComboBox->itemData(currentIndex).toStringList();
    const auto profilePath = profileInfo.at(0);
    for (const auto &p: qAsConst(profiles)) {
        if (p.path == profilePath) {
            profile = p;
            break;
        }
    }
    // Create item and add it to profiles list
    auto *item = new QListWidgetItem();
    // The text from the profile is not up to date if it has been renamed
    item->setText("Proxychains: " + m_ui->proxychainsExtraComboBox->itemText(currentIndex));
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
void FirefoxRunnerConfig::removeExtraOption() {
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

/**
 * Toggle visibility of learn more label
 */
void FirefoxRunnerConfig::learnMoreProxychains() {
    m_ui->proxychainsLearnMoreLabel->setHidden(!m_ui->proxychainsLearnMoreLabel->isHidden());
}

/**
 * Hide proxychains not installed message and persist choice in config
 */
void FirefoxRunnerConfig::hideMessage() {
    config.writeEntry(Config::HideProxychainsMsg, true);
    m_ui->proxychainsNotInstalledWidget->setHidden(true);
}


#include "firefoxprofilerunner_config.moc"
