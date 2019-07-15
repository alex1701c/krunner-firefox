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

    m_ui->profileList->removeButton()->hide();
    m_ui->profileList->addButton()->hide();

    profiles = Profile::getFirefoxProfiles();

    connect(m_ui->profileList, SIGNAL(changed()), this, SLOT(changed()));
    connect(m_ui->refreshProfiles, SIGNAL(clicked(bool)), this, SLOT(changed()));

}

void FirefoxProfileRunnerConfig::load() {

    KCModule::load();

    m_ui->profileList->clear();
    for (const auto &profile:profiles) {
        QString defaultNote = profile.isDefault ? " (default)" : "";
        m_ui->profileList->insertItem(profile.name + defaultNote);
    }

    emit changed(false);
}


void FirefoxProfileRunnerConfig::save() {

    qInfo() << "SAVE!!!";
    // TODO save settings
    emit changed(false);
}

void FirefoxProfileRunnerConfig::defaults() {

    KCModule::defaults();

    // TODO set default values in GUI 
    emit changed(true);
}


#include "firefoxprofilerunner_config.moc"
