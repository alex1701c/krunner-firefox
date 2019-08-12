#include "firefoxprofilerunner_config.h"
#include <KSharedConfig>
#include <KPluginFactory>
#include <krunner/abstractrunner.h>
#include <QtWidgets/QTableWidgetItem>
#include <QtWidgets/QCheckBox>
#include <QDebug>
#include <QtCore/QDir>
#include <QtWidgets/QMessageBox>

K_PLUGIN_FACTORY(FirefoxProfileRunnerConfigFactory,
                 registerPlugin<FirefoxProfileRunnerConfig>("kcm_krunner_firefoxprofilerunner");)

FirefoxProfileRunnerConfigForm::FirefoxProfileRunnerConfigForm(QWidget *parent) : QWidget(parent) {
    setupUi(this);
}

FirefoxProfileRunnerConfig::FirefoxProfileRunnerConfig(QWidget *parent, const QVariantList &args) : KCModule(parent,
                                                                                                             args) {
    m_ui = new FirefoxProfileRunnerConfigForm(this);
    auto *layout = new QGridLayout(this);
    layout->addWidget(m_ui, 0, 0);

    // General settings
    connect(m_ui->registerProfiles, SIGNAL(clicked(bool)), this, SLOT(changed()));
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

    firefoxConfig = KSharedConfig::openConfig(profileManager.firefoxDesktopFile);
    config = KSharedConfig::openConfig("krunnerrc")->group("FirefoxProfileRunner");
}

void FirefoxProfileRunnerConfig::load() {
    m_ui->registerProfiles->setChecked(
            firefoxConfig->group("Settings").readEntry("registerProfiles", "true") == "true");
    m_ui->showIconForPrivateWindow->setChecked(config.readEntry("showIconForPrivateWindow", "true") == "true");
    m_ui->hideDefaultProfile->setChecked(config.readEntry("hideDefaultProfile", "false") == "true");
    m_ui->showAlwaysPrivateWindows->setChecked(config.readEntry("showAlwaysPrivateWindows", "false") == "true");

    profiles = profileManager.syncAndGetCustomProfiles();
    const auto icon = QIcon::fromTheme(
            profileManager.getLaunchCommand().endsWith("firefox") ? "firefox" : "firefox-esr"
    );
    m_ui->profiles->clear();

    QList<QListWidgetItem *> items;
    for (const auto &profile:profiles) {
        auto *item = new QListWidgetItem();
        item->setText(profile.name);
        QList<QVariant> data = {profile.path, profile.isDefault, false, profile.priority};
        item->setData(32, data);

        item->setIcon(icon);
        items.append(item);

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
    showAlwaysPrivateWindows();
    hideDefaultProfile();
    emit changed(false);
}


void FirefoxProfileRunnerConfig::save() {

    firefoxConfig->group("Settings").writeEntry("registerProfiles",
                                                m_ui->registerProfiles->isChecked() ? "true" : "false");
    config.writeEntry("hideDefaultProfile", m_ui->hideDefaultProfile->isChecked() ? "true" : "false");
    config.writeEntry("showIconForPrivateWindow", m_ui->showIconForPrivateWindow->isChecked() ? "true" : "false");
    config.writeEntry("showAlwaysPrivateWindows", m_ui->showAlwaysPrivateWindows->isChecked() ? "true" : "false");

    QList<QListWidgetItem *> items;
    for (int i = 0; i < m_ui->profiles->count(); i++) {
        items.append(m_ui->profiles->item(i));
    }
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

                // Get private window option of window
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
    // New runner instance has latest configuration
    system("kquitapp5 krunner;kstart5 krunner > /dev/null 2&>1");

    emit changed(true);
}

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

void FirefoxProfileRunnerConfig::refreshProfiles() {
    if (!edited) {
        load();
    } else {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Discard changes?",
                                      "Do you want to refresh the current config and discard all unsaved changes",
                                      QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) load();
        else return;
    }
    // Disable Buttons like on initial state
    m_ui->profiles->setFocus();
    if (m_ui->profiles->count() > 0) m_ui->profiles->setCurrentRow(0);
    edited = false;
}

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

void FirefoxProfileRunnerConfig::cancelProfileName() {
    if (m_ui->profiles->currentRow() == -1)return;
    m_ui->editProfileName->setText(m_ui->profiles->currentItem()->text());
    m_ui->editProfileName->setFocus();
}

void FirefoxProfileRunnerConfig::editProfileName() {
    if (m_ui->profiles->currentRow() == -1) return;
    m_ui->editProfileNameApply->setDisabled(
            m_ui->profiles->currentItem()->text() == m_ui->editProfileName->text() ||
            m_ui->editProfileName->text().isEmpty()
    );
    m_ui->editProfileNameCancel->setDisabled(m_ui->profiles->currentItem()->text() == m_ui->editProfileName->text());

}

void FirefoxProfileRunnerConfig::defaults() {
    m_ui->registerProfiles->setChecked(true);
    m_ui->hideDefaultProfile->setChecked(false);
    m_ui->showIconForPrivateWindow->setChecked(true);
    m_ui->showAlwaysPrivateWindows->setChecked(false);
}

void FirefoxProfileRunnerConfig::showAlwaysPrivateWindows() {
    // Hide hide/unhide all private window options
    const int count = m_ui->profiles->count();
    for (int i = 0; i < count; i++) {
        QListWidgetItem *item = m_ui->profiles->item(i);
        if (item->data(32).toList().at(2).toBool()) item->setHidden(!m_ui->showAlwaysPrivateWindows->isChecked());
    }
}

void FirefoxProfileRunnerConfig::hideDefaultProfile() {
    // Hide default profile, private window option of default is handles seperately
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


#include "firefoxprofilerunner_config.moc"
