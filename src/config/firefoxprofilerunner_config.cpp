#include "firefoxprofilerunner_config.h"
#include <KSharedConfig>
#include <KPluginFactory>
#include <krunner/abstractrunner.h>
#include <QtWidgets/QTableWidgetItem>
#include <QtGui/QStandardItem>
#include <QtWidgets/QCheckBox>
#include <QDebug>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QtWidgets>

K_PLUGIN_FACTORY(FirefoxProfileRunnerConfigFactory,
                 registerPlugin<FirefoxProfileRunnerConfig>("kcm_krunner_firefoxprofilerunner");)

FirefoxProfileRunnerConfigForm::FirefoxProfileRunnerConfigForm(QWidget *parent) : QWidget(parent) {
    setupUi(this);
}

FirefoxProfileRunnerConfig::FirefoxProfileRunnerConfig(QWidget *parent, const QVariantList &args) : KCModule(parent, args) {
    m_ui = new FirefoxProfileRunnerConfigForm(this);
    auto *layout = new QGridLayout(this);
    layout->addWidget(m_ui, 0, 0);

    m_ui->editProfileName->setDisabled(true);
    m_ui->editProfileNameApply->setDisabled(true);
    m_ui->editProfileNameCancel->setDisabled(true);
    m_ui->moveUp->setDisabled(true);
    m_ui->moveDown->setDisabled(true);

    connect(m_ui->profiles, SIGNAL(itemSelectionChanged()), this, SLOT(itemSelected()));
    connect(m_ui->profiles, SIGNAL(itemSelectionChanged()), this, SLOT(editProfileName()));
    connect(m_ui->moveUp, SIGNAL(clicked(bool)), this, SLOT(moveUp()));
    connect(m_ui->moveDown, SIGNAL(clicked(bool)), this, SLOT(moveDown()));
    connect(m_ui->editProfileName, SIGNAL(textChanged(QString)), this, SLOT(editProfileName()));
    connect(m_ui->editProfileNameApply, SIGNAL(clicked(bool)), this, SLOT(applyProfileName()));
    connect(m_ui->editProfileNameCancel, SIGNAL(clicked(bool)), this, SLOT(cancelProfileName()));
    connect(m_ui->refreshProfiles, SIGNAL(clicked(bool)), this, SLOT(refreshProfiles()));
    connect(m_ui->refreshProfiles, SIGNAL(clicked(bool)), this, SLOT(changed()));

}

void FirefoxProfileRunnerConfig::load() {

    KCModule::load();

    profiles.clear();
    profiles = Profile::getCustomProfiles();
    m_ui->profiles->clear();

    for (const auto &p:profiles) qInfo() << p.name << p.priority;
    for (const auto &profile:profiles) {
        auto *item = new QListWidgetItem();
        item->setText(profile.name);
        item->setData(1, profile.path);
        m_ui->profiles->addItem(item);
    }

    emit changed(false);
}


void FirefoxProfileRunnerConfig::save() {

    emit changed(false);
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
}

void FirefoxProfileRunnerConfig::moveUp() {
    edited = true;
    const int current = m_ui->profiles->currentRow();
    const auto item = m_ui->profiles->takeItem(current);
    m_ui->profiles->insertItem(current - 1, item);
    m_ui->profiles->setCurrentRow(current - 1);
}

void FirefoxProfileRunnerConfig::moveDown() {
    edited = true;
    const int current = m_ui->profiles->currentRow();
    const auto item = m_ui->profiles->takeItem(current);
    m_ui->profiles->insertItem(current + 1, item);
    m_ui->profiles->setCurrentRow(current + 1);
}

void FirefoxProfileRunnerConfig::applyProfileName() {
    edited = true;
    m_ui->profiles->currentItem()->setText(m_ui->editProfileName->text());
    editProfileName();
    m_ui->profiles->setFocus();

}

void FirefoxProfileRunnerConfig::cancelProfileName() {
    m_ui->editProfileName->setText(m_ui->profiles->currentItem()->text());
    m_ui->editProfileName->setFocus();
}

void FirefoxProfileRunnerConfig::editProfileName() {
    m_ui->editProfileNameApply->setDisabled(m_ui->profiles->currentItem()->text() == m_ui->editProfileName->text());
    m_ui->editProfileNameCancel->setDisabled((m_ui->profiles->currentItem()->text() == m_ui->editProfileName->text()));

}


#include "firefoxprofilerunner_config.moc"
