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

FirefoxProfileRunnerConfig::FirefoxProfileRunnerConfig(QWidget *parent, const QVariantList &args) : KCModule(parent, args) {
    m_ui = new FirefoxProfileRunnerConfigForm(this);
    auto *layout = new QGridLayout(this);
    layout->addWidget(m_ui, 0, 0);

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
}

void FirefoxProfileRunnerConfig::load() {

    KCModule::load();

    profiles.clear();
    profiles = Profile::getCustomProfiles();
    m_ui->profiles->clear();

    for (const auto &profile:profiles) {
        auto *item = new QListWidgetItem();
        item->setText(profile.name);
        item->setData(1, profile.path);
        m_ui->profiles->addItem(item);
    }

    emit changed(false);
}


void FirefoxProfileRunnerConfig::save() {
    QList<QListWidgetItem *> items;
    KSharedConfigPtr firefoxConfig = KSharedConfig::openConfig(QDir::homePath() + "/" + ".local/share/applications/firefox.desktop");
    for (int i = 0; i < m_ui->profiles->count(); i++) {
        items.append(m_ui->profiles->item(i));
    }
    for (auto &profile:profiles) {
        bool matched = false;
        for (int i = 0; i < items.size() && !matched; i++) {
            const auto &item = items.at(i);
            if (item->data(1).toString() == profile.path) {
                if (profile.launchName != item->text()) profile.isEdited = true;
                profile.name = item->text();
                profile.priority = 100 - i;
                profile.writeConfigChanges(firefoxConfig);
                matched = true;
            }
        }
    }
    system("kquitapp5 krunner;kstart5 krunner > /dev/null 2&>1");
    emit changed(true);
}

void FirefoxProfileRunnerConfig::itemSelected() {
    const int idx = m_ui->profiles->currentRow();
    const int count = m_ui->profiles->count();
    m_ui->moveUp->setDisabled(idx == 0);
    m_ui->moveDown->setDisabled(idx == count - 1);
    m_ui->editProfileName->setText(m_ui->profiles->currentItem()->text());
    m_ui->editProfileName->setDisabled(false);
}

void FirefoxProfileRunnerConfig::refreshProfiles() {
    QList<Profile> firefoxProfiles = Profile::getFirefoxProfiles();
    Profile::syncDesktopFile(firefoxProfiles);
    if (!edited) {
        load();
    } else {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Discard changes?",
                                      "Do you want to refresh the current config and discard all unsaved changes",
                                      QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) load();
    }
    // Disable Buttons like on initial state
    m_ui->profiles->setFocus();
    m_ui->profiles->setCurrentRow(0);
    edited = false;
}

void FirefoxProfileRunnerConfig::moveUp() {
    edited = true;
    const int current = m_ui->profiles->currentRow();
    if (current == -1)return;
    const auto item = m_ui->profiles->takeItem(current);
    m_ui->profiles->insertItem(current - 1, item);
    m_ui->profiles->setCurrentRow(current - 1);
}

void FirefoxProfileRunnerConfig::moveDown() {
    edited = true;
    const int current = m_ui->profiles->currentRow();
    if (current == -1)return;
    const auto item = m_ui->profiles->takeItem(current);
    m_ui->profiles->insertItem(current + 1, item);
    m_ui->profiles->setCurrentRow(current + 1);
}

void FirefoxProfileRunnerConfig::applyProfileName() {
    edited = true;
    if (m_ui->profiles->currentRow() == -1)return;
    m_ui->profiles->currentItem()->setText(m_ui->editProfileName->text());
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
    if (m_ui->profiles->currentRow() == -1)return;
    m_ui->editProfileNameApply->setDisabled(
            m_ui->profiles->currentItem()->text() == m_ui->editProfileName->text() || m_ui->editProfileName->text().isEmpty()
    );
    m_ui->editProfileNameCancel->setDisabled(m_ui->profiles->currentItem()->text() == m_ui->editProfileName->text());

}


#include "firefoxprofilerunner_config.moc"
