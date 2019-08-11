#include <KConfigCore/KConfigGroup>
#include <KConfigCore/KSharedConfig>
#include <QDir>
#include <QDebug>
#include "Profile.h"

void
Profile::writeSettings(KSharedConfigPtr firefoxConfig, const QString &profile, const QString &command, int initialPriority) const {
    KConfigGroup profileConfig = firefoxConfig->group(profile);
    if (profileConfig.readEntry("Edited", "false") != "true") {
        profileConfig.writeEntry("Name", this->name.isEmpty() ? this->launchName : this->name);
    }
    profileConfig.writeEntry("LaunchName", this->launchName);
    profileConfig.writeEntry("Edited", profileConfig.readEntry("Edited", "false"));
    profileConfig.writeEntry("Priority", profileConfig.readEntry("Priority", QString::number(initialPriority)));
    profileConfig.writeEntry("Exec", command + " -P \"" + this->launchName + "\"");
}


void Profile::writeConfigChanges(KSharedConfigPtr firefoxConfig) {
    KConfigGroup profileConfig = firefoxConfig->group("Desktop Action new-window-with-profile-" + this->path);
    profileConfig.writeEntry("Name", this->name);
    profileConfig.writeEntry("Edited", true);
    profileConfig.writeEntry("Priority", this->priority);
    profileConfig.writeEntry("PrivateWindowPriority", this->privateWindowPriority);
}

