#ifndef FIREFOXPROFILERUNNER_CONFIG_H
#define FIREFOXPROFILERUNNER_CONFIG_H

#include <QDir>
#include <QFile>

namespace
{
class Config
{
public:
    // General settings
    constexpr static const auto MainGroup = "Config";
    constexpr static const auto HideDefaultProfile = "hideDefaultProfile";
    constexpr static const auto ShowAlwaysPrivateWindows = "showAlwaysPrivateWindows";

    // Profile settings
    constexpr static const auto RegisterNormalWindows = "registerNormalWindows";
    constexpr static const auto RegisterPrivateWindows = "registerPrivateWindows";
    constexpr static const auto PrivateWindowAction = "privateWindowActions";
    // UI settings
    constexpr static const auto GeneralMinimized = "generalMinimized";

    static QString getPrivateWindowIcon()
    {
        if (const QString path = "/usr/share/pixmaps/private_browsing_firefox.svg"; QFileInfo::exists(path)) {
            return path;
        }
        return "view-private";
    }
};
}

#endif // FIREFOXPROFILERUNNER_CONFIG_H
