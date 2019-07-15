#include "firefoxprofilerunner_config.h"
#include <KSharedConfig>
#include <KPluginFactory>
#include <krunner/abstractrunner.h>
#include <QtWidgets/QTableWidgetItem>
#include <QtGui/QStandardItem>
#include <QtWidgets/QCheckBox>
#include <QDebug>

K_PLUGIN_FACTORY(FirefoxProfileRunnerConfigFactory,
                 registerPlugin<FirefoxProfileRunnerConfig>("kcm_krunner_firefoxprofilerunner");)

FirefoxProfileRunnerConfigForm::FirefoxProfileRunnerConfigForm(QWidget *parent) : QWidget(parent) {
    setupUi(this);
}

FirefoxProfileRunnerConfig::FirefoxProfileRunnerConfig(QWidget *parent, const QVariantList &args) : KCModule(parent, args) {
    m_ui = new FirefoxProfileRunnerConfigForm(this);
    auto *layout = new QGridLayout(this);
    layout->addWidget(m_ui, 0, 0);


    profiles = Profile::getFirefoxProfiles();

    m_ui->editProfileName->setDisabled(true);
    m_ui->editProfileNameApply->setDisabled(true);
    m_ui->editProfileNameCancel->setDisabled(true);
    m_ui->moveUp->setDisabled(true);
    m_ui->moveDown->setDisabled(true);

    connect(m_ui->profiles, SIGNAL(itemSelectionChanged()), this, SLOT(itemSelected()));
    connect(m_ui->moveUp, SIGNAL(clicked(bool)), this, SLOT(moveUp()));
    connect(m_ui->moveDown, SIGNAL(clicked(bool)), this, SLOT(moveDown()));
    connect(m_ui->refreshProfiles, SIGNAL(clicked(bool)), this, SLOT(changed()));

}

void FirefoxProfileRunnerConfig::load() {

    KCModule::load();

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

    emit changed(false);
}

void FirefoxProfileRunnerConfig::itemSelected() {
    const int idx = m_ui->profiles->currentRow();
    const int count = m_ui->profiles->count();
    m_ui->moveUp->setDisabled(idx == 0);
    m_ui->moveDown->setDisabled(idx == count - 1);
    m_ui->editProfileName->setText(m_ui->profiles->currentItem()->text());
    m_ui->editProfileName->setDisabled(false);
    m_ui->editProfileNameApply->setDisabled(false);
    m_ui->editProfileNameCancel->setDisabled(false);
}

void FirefoxProfileRunnerConfig::moveUp() {
    const int current = m_ui->profiles->currentRow();
    const auto item = m_ui->profiles->takeItem(current);
    m_ui->profiles->insertItem(current - 1, item);
    m_ui->profiles->setCurrentRow(current - 1);
}

void FirefoxProfileRunnerConfig::moveDown() {
    const int current = m_ui->profiles->currentRow();
    const auto item = m_ui->profiles->takeItem(current);
    m_ui->profiles->insertItem(current + 1, item);
    m_ui->profiles->setCurrentRow(current + 1);
}


#include "firefoxprofilerunner_config.moc"
