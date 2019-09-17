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

    // Write settings for proxychains normal window
    KConfigGroup proxychainsNormalConfig = firefoxConfig->group(
            "Desktop Action new-proxychains-normal-window-with-profile-" + profile);
    if (proxychainsNormalConfig.readEntry("Edited", "false") != "true") {
        proxychainsNormalConfig.writeEntry("Name", "Proychains: " + (this->name.isEmpty() ? this->launchName : this->name));
    }
    proxychainsNormalConfig.writeEntry("LaunchName", this->launchName);
    proxychainsNormalConfig.writeEntry("Edited", proxychainsNormalConfig.readEntry("Edited", "false"));
    // TODO New instance option
    proxychainsNormalConfig.writeEntry("Exec", "proxychains" + command + " -P \"" + this->launchName + "\"");

    // Write settings for proxychains private window
    KConfigGroup proxychainsPrivateConfig = firefoxConfig->group(
            "Desktop Action new-proxychains-private-window-with-profile-" + profile);
    if (proxychainsPrivateConfig.readEntry("Edited", "false") != "true") {
        proxychainsPrivateConfig.writeEntry("Name", "Proychains: " + (this->name.isEmpty() ? this->launchName : this->name));
    }
    proxychainsPrivateConfig.writeEntry("LaunchName", this->launchName);
    proxychainsPrivateConfig.writeEntry("Edited", proxychainsPrivateConfig.readEntry("Edited", "false"));
    proxychainsPrivateConfig.writeEntry("Exec", "proxychains4" + command + " -P \"" + this->launchName + "\" -private-window");
}

/**
 * Write changes from the config dialog
 * @param firefoxConfig
 */
void Profile::writeConfigChanges(KSharedConfigPtr firefoxConfig, const QString &forceNewInstance) {
    // TODO Update launch commands, check new instance
    // General config/normal launch option
    KConfigGroup profileConfig = firefoxConfig->group("Desktop Action new-window-with-profile-" + this->path);
    profileConfig.writeEntry("Name", this->name);
    profileConfig.writeEntry("Edited", true);
    profileConfig.writeEntry("Priority", this->priority);
    if (this->launchNormalWindowWithProxychains) {
        profileConfig.writeEntry("Exec", "proxychains4 " + this->launchCommand +
                                         " -P \"" + this->launchName + "\"" + forceNewInstance);
    } else {
        profileConfig.writeEntry("Exec", this->launchCommand + " -P \"" + this->launchName + "\"");
    }
    profileConfig.writeEntry("PrivateWindowPriority", this->privateWindowPriority);
    profileConfig.writeEntry("ProxychainsNormalWindowPriority", this->extraNormalWindowProxychainsOptionPriority);
    profileConfig.writeEntry("ProxychainsPrivateWindowPriority", this->extraPrivateWindowProxychainsOptionPriority);


    // Private window launch options
    KConfigGroup privateDesktopAction = firefoxConfig->group("Desktop Action new-private-window-with-profile-" + this->path);
    privateDesktopAction.writeEntry("Name", this->name);
    if (this->launchPrivateWindowWithProxychains) {
        privateDesktopAction.writeEntry("Exec", "proxychains4 " + this->launchCommand + " -P \""
                                                + this->launchName + "\" -private-window" + forceNewInstance);
    } else {
        privateDesktopAction.writeEntry("Exec", this->launchCommand + " -P \"" + this->launchName + "\" -private-window");
    }

    KConfigGroup proxychainsNormalConfig = firefoxConfig->group(
            "Desktop Action new-proxychains-normal-window-with-profile-" + this->path);
    proxychainsNormalConfig.writeEntry("Name", "Proxychains: " + this->name);
    proxychainsNormalConfig.writeEntry("Exec", "proxychains4 " + this->launchCommand +
                                               " -P \"" + this->launchName + "\"" + forceNewInstance);


    KConfigGroup proxychainsPrivateConfig = firefoxConfig->group(
            "Desktop Action new-proxychains-normal-window-with-profile-" + this->path);
    proxychainsPrivateConfig.writeEntry("Name", "Proxychains: " + this->name);
    proxychainsNormalConfig.writeEntry("Exec", "proxychains4 " + this->launchCommand +
                                               " -P \"" + this->launchName + "\" -private-window" + forceNewInstance);

}

