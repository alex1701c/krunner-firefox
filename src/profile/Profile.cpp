#include <KConfigCore/KConfigGroup>
#include <KConfigCore/KSharedConfig>
#include <QDir>
#include <QDebug>
#include "Profile.h"

/**
 * Write/Overwrite the config of the Desktop Actions for normal/private windows
 * @param firefoxConfig
 * @param profile
 * @param command
 * @param initialPriority
 */
void
Profile::writeSettings(KSharedConfigPtr firefoxConfig, const QString &profile, const QString &command, int initialPriority) const {
    // Write settings for normal window
    KConfigGroup profileNormalConfig = firefoxConfig->group("Desktop Action new-window-with-profile-" + profile);
    if (profileNormalConfig.readEntry("Edited", "false") != "true") {
        profileNormalConfig.writeEntry("Name", this->name.isEmpty() ? this->launchName : this->name);
    }
    profileNormalConfig.writeEntry("LaunchName", this->launchName);
    profileNormalConfig.writeEntry("Edited", profileNormalConfig.readEntry("Edited", "false"));
    profileNormalConfig.writeEntry("Priority", profileNormalConfig.readEntry("Priority", QString::number(initialPriority)));
    profileNormalConfig.writeEntry("Exec", command + " -P \"" + this->launchName + "\"");

    // Write settings for private window
    KConfigGroup profilePrivateConfig = firefoxConfig->group("Desktop Action new-private-window-with-profile-" + profile);
    if (profilePrivateConfig.readEntry("Edited", "false") != "true") {
        profilePrivateConfig.writeEntry("Name", this->name.isEmpty() ? this->launchName : this->name);
    }
    profilePrivateConfig.writeEntry("LaunchName", this->launchName);
    profilePrivateConfig.writeEntry("Icon", "/usr/share/icons/private_browsing_firefox.svg");
    profilePrivateConfig.writeEntry("Edited", profileNormalConfig.readEntry("Edited", "false"));
    profilePrivateConfig.writeEntry("Exec", command + " -P \"" + this->launchName + "\" -private-window");
}

/**
 * Write changes from the config dialog
 * @param firefoxConfig
 */
void Profile::writeConfigChanges(KSharedConfigPtr firefoxConfig) {
    KConfigGroup profileConfig = firefoxConfig->group("Desktop Action new-window-with-profile-" + this->path);
    profileConfig.writeEntry("Name", this->name);
    profileConfig.writeEntry("Edited", true);
    profileConfig.writeEntry("Priority", this->priority);
    profileConfig.writeEntry("PrivateWindowPriority", this->privateWindowPriority);

    KConfigGroup profilePrivateConfig = firefoxConfig->group("Desktop Action new-private-window-with-profile-" + this->path);
    profilePrivateConfig.writeEntry("Name", this->name);
}

