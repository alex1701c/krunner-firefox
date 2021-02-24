/**
 *  This file is only for the Tests !
 */

#ifndef FIREFOXPROFILERUNNER_CONFIG_H
#define FIREFOXPROFILERUNNER_CONFIG_H

#include <QDir>
namespace {
    class Config {
    public:
        // General settings
        static const QString ConfigDir;
        static const QString ConfigFile;
        constexpr static const auto MainGroup = "Config";
        constexpr static const auto HideDefaultProfile = "hideDefaultProfile";
        constexpr static const auto ShowAlwaysPrivateWindows = "showAlwaysPrivateWindows";
        constexpr static const auto ProxychainsForceNewInstance = "proxychainsForceNewInstance";
        constexpr static const auto ProxychainsIntegration = "proxychainsIntegration";

        // Profile settings
        constexpr static const auto RegisterNormalWindows = "registerNormalWindows";
        constexpr static const auto RegisterPrivateWindows = "registerPrivateWindows";
        constexpr static const auto PrivateWindowAction = "privateWindowActions";
        constexpr static const auto ShowProxychainsOptionsGlobally = "showProxychainsOptionsGlobally";
        // UI settings
        constexpr static const auto ProxychainsMinimized = "proxychainsMinimized";
        constexpr static const auto HideProxychainsMsg = "hide_not_installed_message";
        constexpr static const auto GeneralMinimized = "generalMinimized";
    };
}

const QString Config::ConfigDir = QDir::homePath() + "/.config/krunnerplugins/";
const QString Config::ConfigFile = Config::ConfigDir + "firefoxprofilerunnerrc";

#endif //FIREFOXPROFILERUNNER_CONFIG_H
