#include <KConfigCore/KConfigGroup>
#include <KConfigCore/KSharedConfig>
#include <QDir>
#include <QDebug>
#include "Profile.h"

/**
 * Write/Overwrite the config of the Desktop Actions for normal/private windows
 * @param firefoxConfig
 * @param initialPriority
 */
void Profile::writeSettings(KSharedConfigPtr firefoxConfig, int initialPriority) const {
    // Write settings for normal window
    KConfigGroup profileNormalConfig = firefoxConfig->group("Desktop Action new-window-with-profile-" + this->path);
    if (profileNormalConfig.readEntry("Edited", "false") != "true") {
        profileNormalConfig.writeEntry("Name", this->name.isEmpty() ? this->launchName : this->name);
    }
    profileNormalConfig.writeEntry("LaunchName", this->launchName);
    profileNormalConfig.writeEntry("Edited", profileNormalConfig.readEntry("Edited", "false"));
    profileNormalConfig.writeEntry("Priority", profileNormalConfig.readEntry("Priority", QString::number(initialPriority)));
    profileNormalConfig.writeEntry("Exec", this->launchCommand + " -P \"" + this->launchName + "\"");

    // Write settings for private window
    KConfigGroup profilePrivateConfig = firefoxConfig->group("Desktop Action new-private-window-with-profile-" + this->path);
    if (profilePrivateConfig.readEntry("Edited", "false") != "true") {
        profilePrivateConfig.writeEntry("Name", this->name.isEmpty() ? this->launchName : this->name);
    }
    profilePrivateConfig.writeEntry("LaunchName", this->launchName);
    profilePrivateConfig.writeEntry("Icon", "/usr/share/icons/private_browsing_firefox.svg");
    profilePrivateConfig.writeEntry("Edited", profileNormalConfig.readEntry("Edited", "false"));
    profilePrivateConfig.writeEntry("Exec", this->launchCommand + " -P \"" + this->launchName + "\" -private-window");

}

/**
 * Write changes from the config dialog to the firefox.desktop file
 * @param firefoxConfig
 */
void Profile::writeConfigChanges(KSharedConfigPtr firefoxConfig, const QString &forceNewInstance) const {
    // General config/normal launch option
    KConfigGroup profileConfig = firefoxConfig->group("Desktop Action new-window-with-profile-" + this->path);
    profileConfig.writeEntry("Name", this->name);
    profileConfig.writeEntry("Edited", true);
    if (this->launchNormalWindowWithProxychains) {
        profileConfig.writeEntry("Exec", "proxychains4 " + this->launchCommand +
                                         " -P \"" + this->launchName + "\"" + forceNewInstance);
    } else {
        profileConfig.writeEntry("Exec", this->launchCommand + " -P \"" + this->launchName + "\"");
    }
    profileConfig.writeEntry("Priority", this->priority);
    profileConfig.writeEntry("PrivateWindowPriority", this->privateWindowPriority);
    profileConfig.writeEntry("ProxychainsNormalWindowOption", this->extraNormalWindowProxychainsLaunchOption);
    profileConfig.writeEntry("ProxychainsNormalWindowPriority", this->extraNormalWindowProxychainsOptionPriority);
    profileConfig.writeEntry("ProxychainsPrivateWindowOption", this->extraPrivateWindowProxychainsLaunchOption);
    profileConfig.writeEntry("ProxychainsPrivateWindowPriority", this->extraPrivateWindowProxychainsOptionPriority);
    profileConfig.writeEntry("LaunchNormalWindowWithProxychains", this->launchNormalWindowWithProxychains);
    profileConfig.writeEntry("LaunchPrivateWindowWithProxychains", this->launchPrivateWindowWithProxychains);

    // Private window launch options
    KConfigGroup privateDesktopAction = firefoxConfig->group("Desktop Action new-private-window-with-profile-" + this->path);
    privateDesktopAction.writeEntry("Name", this->name);
    if (this->launchPrivateWindowWithProxychains) {
        privateDesktopAction.writeEntry("Exec", "proxychains4 " + this->launchCommand + " -P \""
                                                + this->launchName + "\" -private-window" + forceNewInstance);
    } else {
        privateDesktopAction.writeEntry("Exec", this->launchCommand + " -P \"" + this->launchName + "\" -private-window");
    }

    // Extra proxychains normal window option
    KConfigGroup proxychainsNormalConfig = firefoxConfig->group(
            "Desktop Action new-proxychains-normal-window-with-profile-" + this->path);
    if (this->extraNormalWindowProxychainsLaunchOption) {
        proxychainsNormalConfig.writeEntry("Name", "Proxychains: " + this->name);
        proxychainsNormalConfig.writeEntry("Exec", "proxychains4 " + this->launchCommand +
                                                   " -P \"" + this->launchName + "\"" + forceNewInstance);
    } else {
        proxychainsNormalConfig.deleteGroup();
    }

    // Extra proxychains private window option
    KConfigGroup proxychainsPrivateConfig = firefoxConfig->group(
            "Desktop Action new-proxychains-private-window-with-profile-" + this->path);
    if (this->extraPrivateWindowProxychainsLaunchOption) {
        proxychainsPrivateConfig.writeEntry("Name", "Proxychains: " + this->name);
        proxychainsPrivateConfig.writeEntry("Icon", "/usr/share/icons/private_browsing_firefox.svg");
        proxychainsPrivateConfig.writeEntry("Exec", "proxychains4 " + this->launchCommand + " -P \"" +
                                                    this->launchName + "\" -private-window" + forceNewInstance);
    } else {
        proxychainsPrivateConfig.deleteGroup();
    }
}

void Profile::toString() const {
    qInfo() << "name: " << this->name << " launchCommand: " << this->launchCommand << " launchName: "
            << this->launchName << " path: " << this->path << " priority: "
            << this->priority << " isDefault: " << this->isDefault << " isEdited: " << this->isEdited
            << " launchNormalWindowWithProxychains: " << this->launchNormalWindowWithProxychains
            << " extraNormalWindowProxychainsLaunchOption: " << this->extraNormalWindowProxychainsLaunchOption
            << " extraNormalWindowProxychainsOptionPriority: " << this->extraNormalWindowProxychainsOptionPriority
            << " privateWindowPriority: " << this->privateWindowPriority << " launchPrivateWindowWithProxychains: "
            << this->launchPrivateWindowWithProxychains << " extraPrivateWindowProxychainsLaunchOption: "
            << this->extraPrivateWindowProxychainsLaunchOption << " extraPrivateWindowProxychainsOptionPriority: "
            << this->extraPrivateWindowProxychainsOptionPriority << "\n";
}

