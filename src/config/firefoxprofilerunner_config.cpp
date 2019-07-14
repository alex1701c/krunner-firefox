#include "firefoxprofilerunner_config.h"
#include <KSharedConfig>
#include <KPluginFactory>
#include <krunner/abstractrunner.h>

K_PLUGIN_FACTORY(FirefoxProfileRunnerConfigFactory, registerPlugin<FirefoxProfileRunnerConfig>("kcm_krunner_firefoxprofilerunner");)

FirefoxProfileRunnerConfigForm::FirefoxProfileRunnerConfigForm(QWidget *parent) : QWidget(parent) {
    setupUi(this);
}

FirefoxProfileRunnerConfig::FirefoxProfileRunnerConfig(QWidget *parent, const QVariantList &args) : KCModule(parent, args) {
    m_ui = new FirefoxProfileRunnerConfigForm(this);
    auto *layout = new QGridLayout(this);
    layout->addWidget(m_ui, 0, 0);

    // TODO connect signals

    load();
}

void FirefoxProfileRunnerConfig::load() {

    KCModule::load();
    
    // TODO load settings into GUI
    emit changed(false);
}


void FirefoxProfileRunnerConfig::save() {

    KCModule::save();

    // TODO save settings
    emit changed(false);
}

void FirefoxProfileRunnerConfig::defaults() {

    KCModule::defaults();

    // TODO set default values in GUI 
    emit changed(true);
}


#include "firefoxprofilerunner_config.moc"
