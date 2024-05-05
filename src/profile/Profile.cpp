#include "Profile.h"
#include <KConfigGroup>
#include <KSharedConfig>
#include <QDebug>
#include <QDir>

/**
 * Write/Overwrite the config of the Desktop Actions for normal/private windows
 * @param firefoxConfig
 * @param initialPriority
 */
void Profile::writeSettings(KSharedConfigPtr firefoxConfig, int initialPriority) const
{
    // Write settings for normal window
    KConfigGroup profileNormalConfig = firefoxConfig->group("Desktop Action new-window-with-profile-" + this->path);
    if (!profileNormalConfig.readEntry("Edited", false)) {
        profileNormalConfig.writeEntry("Name", this->name.isEmpty() ? this->launchName : this->name);
    }
    profileNormalConfig.writeEntry("LaunchName", this->launchName);
    profileNormalConfig.writeEntry("Edited", profileNormalConfig.readEntry("Edited", "false"));
    profileNormalConfig.writeEntry("Priority", profileNormalConfig.readEntry("Priority", QString::number(initialPriority)));
    profileNormalConfig.writeEntry("Exec", this->launchCommand + " -P \"" + this->launchName + "\"");

    // Write settings for private window
    KConfigGroup profilePrivateConfig = firefoxConfig->group("Desktop Action new-private-window-with-profile-" + this->path);
    if (!profilePrivateConfig.readEntry("Edited", false)) {
        profilePrivateConfig.writeEntry("Name", this->name.isEmpty() ? this->launchName : this->name);
    }
    profilePrivateConfig.writeEntry("LaunchName", this->launchName);
    profilePrivateConfig.writeEntry("Icon", "private_browsing_firefox");
    profilePrivateConfig.writeEntry("Edited", profileNormalConfig.readEntry("Edited", false));
    profilePrivateConfig.writeEntry("Exec", this->launchCommand + " -P \"" + this->launchName + "\" -private-window");
}

/**
 * Write changes from the config dialog to the firefox.desktop file
 * @param firefoxConfig
 */
void Profile::writeConfigChanges(KSharedConfigPtr firefoxConfig, const QString &forceNewInstance) const
{
    // General config/normal launch option
    KConfigGroup profileConfig = firefoxConfig->group("Desktop Action new-window-with-profile-" + this->path);
    profileConfig.writeEntry("Name", this->name);
    profileConfig.writeEntry("Edited", true);
    profileConfig.writeEntry("Exec", this->launchCommand + " -P \"" + this->launchName + "\"");
    profileConfig.writeEntry("Priority", this->priority);
    profileConfig.writeEntry("PrivateWindowPriority", this->privateWindowPriority);

    // Private window launch options
    KConfigGroup privateDesktopAction = firefoxConfig->group("Desktop Action new-private-window-with-profile-" + this->path);
    privateDesktopAction.writeEntry("Name", this->name);
    privateDesktopAction.writeEntry("Exec", this->launchCommand + " -P \"" + this->launchName + "\" -private-window");
}

void Profile::toString() const
{
    qInfo() << "name: " << this->name << " launchCommand: " << this->launchCommand << " launchName: " << this->launchName << " path: " << this->path
            << " priority: " << this->priority << " isDefault: " << this->isDefault << " isEdited: " << this->isEdited
            << " privateWindowPriority: " << this->privateWindowPriority;
}
