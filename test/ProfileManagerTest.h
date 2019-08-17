#include <QTest>
#include "../src/profile/ProfileManager.h"
#include <iostream>
#include <KSharedConfig>
#include <KConfigGroup>
#include "helper.h"

class ProfileManagerTest : public QObject {
Q_OBJECT


private slots:

    /**
     * Test if the profiles are read correctly
     * The profiles file that is read contains an Install.* group
     */
    static void testReadProfilesFromWithInstallGroup() {
        ProfileManager manager;
        manager.firefoxProfilesIniPath = QDir::currentPath() + "/resources/profiles_install.ini";
        manager.defaultPath = manager.getDefaultProfilePath();
        QList<Profile> rawProfiles = manager.getFirefoxProfiles();

        QVERIFY(QFile::exists(manager.firefoxProfilesIniPath));
        QCOMPARE(rawProfiles.size(), 2);
        QCOMPARE(manager.getDefaultProfilePath(), "snytc8pd.default-release-1");
    }


    /**
     * Test if the profiles are read correctly if the default information
     * is contained within a Profile.* group
     */
    static void testReadProfilesFromWithoutInstallGroup() {
        ProfileManager manager;
        manager.firefoxProfilesIniPath = QDir::currentPath() + "/resources/profiles.ini";
        manager.defaultPath = manager.getDefaultProfilePath();
        QList<Profile> rawProfiles = manager.getFirefoxProfiles();

        QVERIFY(QFile::exists(manager.firefoxProfilesIniPath));
        QCOMPARE(rawProfiles.size(), 2);
        QCOMPARE(manager.getDefaultProfilePath(), "x4wpq9zm.default");
    }

    /**
     * Read profiles and sync them with the firefox.desktop file
     * Test if the default profile is read correctly and if the registering of
     * the Desktop Actions works correctly
     */
    static void testSyncAndReadInstalledProfile() {
        ProfileManager manager;
        manager.firefoxProfilesIniPath = QDir::currentPath() + "/resources/profiles_install.ini";
        manager.defaultPath = manager.getDefaultProfilePath();

        QFile::copy(QDir::currentPath() + "/resources/firefox.desktop", QDir::currentPath() + "/resources/firefox-copy.desktop");
        manager.firefoxDesktopFile = QDir::currentPath() + "/resources/firefox-copy.desktop";
        const auto firefoxConfig = KSharedConfig::openConfig(manager.firefoxDesktopFile);
        QList<Profile> rawProfiles = manager.getFirefoxProfiles();

        // Check if Desktop Actions are created and if they are registered
        QCOMPARE(rawProfiles.size(), 2);
        manager.syncDesktopFile(rawProfiles, firefoxConfig);
        QCOMPARE(firefoxConfig->groupList().filter(QRegExp(R"(^Desktop Action new-(?:private-)?window-with-profile)")).size(), 4);
        manager.changeProfileRegistering(true, true, firefoxConfig);
        QCOMPARE(getSplitCount(firefoxConfig->group("Desktop Entry").readEntry("Actions")), 6);

        // Unregister private windows
        manager.changeProfileRegistering(true, false, firefoxConfig);
        QCOMPARE(getSplitCount(firefoxConfig->group("Desktop Entry").readEntry("Actions")), 4);
        QCOMPARE(firefoxConfig->group("Desktop Entry").readEntry("Actions").split(";")
                         .filter(QRegExp(R"(^new-private-window-with-profile)")).count(), 0);

        // Unregister all windows
        manager.changeProfileRegistering(false, false, firefoxConfig);
        QCOMPARE(getSplitCount(firefoxConfig->group("Desktop Entry").readEntry("Actions")), 2);

        // Register all options
        manager.changeProfileRegistering(true, true, firefoxConfig);
        QCOMPARE(getSplitCount(firefoxConfig->group("Desktop Entry").readEntry("Actions")), 6);

        // Check if profiles are read correctly
        QList<Profile> customProfiles = manager.getCustomProfiles(firefoxConfig);
        QCOMPARE(customProfiles.count(), 2);
        for (const auto &profile:customProfiles) {
            if (profile.name == "Alex") {
                QVERIFY(profile.isDefault);
            }
        }
    }

};

QTEST_MAIN(ProfileManagerTest)
