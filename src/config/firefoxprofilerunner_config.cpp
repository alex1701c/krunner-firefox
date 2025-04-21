#include "firefoxprofilerunner_config.h"
#include <Config.h>
#include <KPluginFactory>
#include <KSharedConfig>
#include <QCheckBox>
#include <QDebug>
#include <QMessageBox>
#include <QProcess>

#include "config_types.h"

K_PLUGIN_CLASS(FirefoxRunnerConfig)

inline ProfileData widgetData(const QListWidgetItem *item)
{
    const QVariant &var = item->data(Qt::UserRole);
    return var.value<ProfileData>();
}

FirefoxProfileRunnerConfigForm::FirefoxProfileRunnerConfigForm(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);
}

FirefoxRunnerConfig::FirefoxRunnerConfig(QObject *parent, const QVariantList &)
    : KCModule(parent)
{
    m_ui = new FirefoxProfileRunnerConfigForm(widget());
    auto *layout = new QGridLayout(widget());
    layout->addWidget(m_ui, 0, 0);

    connectSignals();

    firefoxConfig = KSharedConfig::openConfig(profileManager.firefoxDesktopFile);
    config = KSharedConfig::openConfig("krunnerrc")->group("Runners").group("firefoxprofilerunner");
    config.config()->reparseConfiguration();
    firefoxIcon = QIcon::fromTheme(profileManager.launchCommand.endsWith("firefox") ? "firefox" : "firefox-esr");
}

void FirefoxRunnerConfig::load()
{
    m_ui->registerPrivateWindows->setChecked(config.readEntry(Config::RegisterPrivateWindows, true));
    m_ui->registerNormalWindows->setChecked(config.readEntry(Config::RegisterNormalWindows, true));
    m_ui->hideDefaultProfile->setChecked(config.readEntry(Config::HideDefaultProfile, false));
    m_ui->showAlwaysPrivateWindows->setChecked(config.readEntry(Config::ShowAlwaysPrivateWindows, true));
    m_ui->privateWindowAction->setChecked(config.readEntry(Config::PrivateWindowAction, false));
    privateWindowsAsActionsChanged();

    profileManager = ProfileManager();
    profiles = profileManager.syncAndGetCustomProfiles(config, forceProfileSync);
    forceProfileSync = false;
    m_ui->profiles->clear();

    const QIcon privateWindowIcon = QIcon::fromTheme(Config::getPrivateWindowIcon());
    QList<QListWidgetItem *> items;
    for (int i = 0; i < profiles.count(); ++i) {
        auto &profile = profiles.at(i);
        // Normal window
        auto *item = new QListWidgetItem();
        item->setText(profile.name);
        auto data = QVariant::fromValue(ProfileData{profile.path, profile.isDefault, ProfileType::Normal, profile.priority});
        item->setData(Qt::UserRole, data);
        item->setIcon(firefoxIcon);
        items.append(item);
        // Private window
        auto *item2 = new QListWidgetItem();
        item2->setText(profile.name);
        auto data2 = QVariant::fromValue(ProfileData{profile.path, profile.isDefault, ProfileType::Private, profile.privateWindowPriority});
        item2->setData(Qt::UserRole, data2);
        item2->setIcon(privateWindowIcon);
        items.append(item2);
    }
    std::sort(items.begin(), items.end(), [](const QListWidgetItem *item1, const QListWidgetItem *item2) -> bool {
        return widgetData(item1).priority > widgetData(item2).priority;
    });
    for (const auto &item : qAsConst(items)) {
        m_ui->profiles->addItem(item);
    }

    // Initial validation
    showAlwaysPrivateWindows();
    if (config.readEntry(Config::GeneralMinimized, false)) {
        toggleGeneralConfigVisibility();
    }

    hideDefaultProfile();
}

void FirefoxRunnerConfig::save()
{
    // Write general settings
    config.writeEntry(Config::GeneralMinimized, m_ui->firefoxGeneralConfigWidget->isHidden());
    config.writeEntry(Config::RegisterNormalWindows, m_ui->registerNormalWindows->isChecked());
    config.writeEntry(Config::RegisterPrivateWindows, m_ui->registerPrivateWindows->isChecked());
    config.writeEntry(Config::PrivateWindowAction, m_ui->privateWindowAction->isChecked());
    config.writeEntry(Config::HideDefaultProfile, m_ui->hideDefaultProfile->isEnabled() && m_ui->hideDefaultProfile->isChecked());
    config.writeEntry(Config::ShowAlwaysPrivateWindows, m_ui->showAlwaysPrivateWindows->isEnabled() && m_ui->showAlwaysPrivateWindows->isChecked());

    // Organize entries in a map <profilePath, <type, item> >
    QMap<QString, QMap<ProfileType, QListWidgetItem *>> organizedItems;
    for (int i = 0; i < m_ui->profiles->count(); ++i) {
        auto *item = m_ui->profiles->item(i);
        auto data = widgetData(item);
        data.priority = 100 - i;
        item->setData(Qt::UserRole, QVariant::fromValue(data));
        auto newData = organizedItems.value(data.path);
        newData.insert(data.type, item);
        organizedItems.insert(data.path, newData);
    }
    // Sync profiles with entries from map
    for (auto &profile : profiles) {
        // Reset optional values to false/0
        profile.privateWindowPriority = 0;

        auto itemMap = organizedItems.value(profile.path);
        for (const auto &type : itemMap.keys()) {
            const auto item = itemMap.value(type);
            const auto itemData = widgetData(item);
            if (type == Normal) {
                profile.name = item->text();
                profile.priority = itemData.priority;
            } else if (type == Private) {
                profile.privateWindowPriority = itemData.priority;
            }
        }
        profile.writeConfigChanges(firefoxConfig);
    }

    // Change registering based on new settings
    profileManager.changeProfileRegistering(m_ui->registerNormalWindows->isChecked(), m_ui->registerPrivateWindows->isChecked(), firefoxConfig);
    // New runner instance has latest configuration
    QProcess::startDetached("bash",
                            QStringList() << "-c"
                                          << "kquitapp5 krunner;kstart5 krunner > /dev/null");
}

void FirefoxRunnerConfig::defaults()
{
    m_ui->registerNormalWindows->setChecked(true);
    m_ui->registerPrivateWindows->setChecked(true);
    m_ui->hideDefaultProfile->setChecked(false);
    m_ui->showAlwaysPrivateWindows->setChecked(true);
    m_ui->privateWindowAction->setChecked(false);
    privateWindowsAsActionsChanged();

    hideDefaultProfile();

    markAsChanged();
}

void FirefoxRunnerConfig::connectSignals()
{
    const auto changedSlotPointer = &FirefoxRunnerConfig::markAsChanged;
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
}

/**
 * Enable/Disable Up/Down buttons
 */
void FirefoxRunnerConfig::itemSelected()
{
    const int idx = m_ui->profiles->currentRow();
    // When the checkboxes are clicked but no item has been selected
    if (idx == -1) {
        return;
    }
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
void FirefoxRunnerConfig::refreshProfiles()
{
    if (!edited) {
        forceProfileSync = true;
        load();
    } else {
        QMessageBox::StandardButton reply = //
            QMessageBox::question(widget(),
                                  "Discard changes?",
                                  "Do you want to refresh the current config and discard all unsaved changes",
                                  QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            forceProfileSync = true;
            load();
        } else {
            return;
        }
    }
    // Disable Buttons like on initial state
    m_ui->profiles->setFocus();
    if (m_ui->profiles->count() > 0) {
        m_ui->profiles->setCurrentRow(0);
    }
    edited = false;
    hideDefaultProfile();
}

void FirefoxRunnerConfig::privateWindowsAsActionsChanged()
{
    m_ui->hideDefaultProfile->setDisabled(m_ui->privateWindowAction->isChecked());
    m_ui->showAlwaysPrivateWindows->setDisabled(m_ui->privateWindowAction->isChecked());
    showAlwaysPrivateWindows();
    hideDefaultProfile();
}

/**
 * Move item up
 */
void FirefoxRunnerConfig::moveUp()
{
    edited = true;
    const int current = m_ui->profiles->currentRow();
    if (current < 1) {
        return;
    }

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
void FirefoxRunnerConfig::moveDown()
{
    edited = true;
    const int current = m_ui->profiles->currentRow();
    const int count = m_ui->profiles->count();
    if (current == -1) {
        return;
    }

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
void FirefoxRunnerConfig::applyProfileName()
{
    edited = true;
    const QString editedText = m_ui->editProfileName->text();
    if (m_ui->profiles->currentRow() == -1) {
        return;
    }
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
void FirefoxRunnerConfig::cancelProfileName()
{
    if (m_ui->profiles->currentRow() != -1) {
        m_ui->editProfileName->setText(m_ui->profiles->currentItem()->text());
        m_ui->editProfileName->setFocus();
    }
}

/**
 * Validate buttons to cancel/apply the changes to profile name
 */
void FirefoxRunnerConfig::editProfileName()
{
    if (m_ui->profiles->currentRow() != -1) {
        m_ui->editProfileNameApply->setDisabled(m_ui->profiles->currentItem()->text() == m_ui->editProfileName->text()
                                                || m_ui->editProfileName->text().isEmpty());
        m_ui->editProfileNameCancel->setDisabled(m_ui->profiles->currentItem()->text() == m_ui->editProfileName->text());
    }
}

/**
 * Hide hide/unhide all private window options
 */
void FirefoxRunnerConfig::showAlwaysPrivateWindows()
{
    const bool hide = !m_ui->showAlwaysPrivateWindows->isChecked() || !m_ui->showAlwaysPrivateWindows->isEnabled();
    for (int i = 0; i < m_ui->profiles->count(); ++i) {
        QListWidgetItem *item = m_ui->profiles->item(i);
        if (widgetData(item).type == ProfileType::Private) {
            item->setHidden(hide);
        }
    }
}

/**
 * Hide default profile from profiles list if checkbox is checked
 */
void FirefoxRunnerConfig::hideDefaultProfile()
{
    const bool hide = m_ui->hideDefaultProfile->isChecked() && m_ui->hideDefaultProfile->isEnabled();
    for (int i = 0; i < m_ui->profiles->count(); ++i) {
        QListWidgetItem *item = m_ui->profiles->item(i);
        const auto data = widgetData(item);
        if (data.type == ProfileType::Normal && data.isDefault) {
            item->setHidden(hide);
            break;
        }
    }
}

/**
 * Hide/Show the general config config widgets
 */
void FirefoxRunnerConfig::toggleGeneralConfigVisibility()
{
    bool hide = !m_ui->firefoxGeneralConfigWidget->isHidden();
    m_ui->generalConfigToggleHidePushButton->setIcon(hide ? downIcon : upIcon);
    m_ui->firefoxGeneralConfigWidget->setHidden(hide);
}

#include "firefoxprofilerunner_config.moc"
#include "moc_firefoxprofilerunner_config.cpp"
