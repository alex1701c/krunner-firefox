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
        QFile::copy(QDir::currentPath() + "/resources/profiles_install.ini", QDir::currentPath() + "/resources/_profiles_install.ini");

        ProfileManager manager;
        manager.firefoxProfilesIniPath = QDir::currentPath() + "/resources/_profiles_install.ini";
        manager.defaultPath = manager.getDefaultProfilePath();

        QFile::remove(QDir::currentPath() + "/resources/firefox-copy.desktop");
        QFile::copy(QDir::currentPath() + "/resources/firefox.desktop", QDir::currentPath() + "/resources/firefox-copy.desktop");
        manager.firefoxDesktopFile = QDir::currentPath() + "/resources/firefox-copy.desktop";
        const auto firefoxConfig = KSharedConfig::openConfig(manager.firefoxDesktopFile);
        QList<Profile> rawProfiles = manager.getFirefoxProfiles();
        KConfigGroup config = KSharedConfig::openConfig(QDir::currentPath() + "tmp")->group("Config");

        // Check if Desktop Actions are created and if they are registered
        QCOMPARE(rawProfiles.size(), 2);
        manager.syncDesktopFile(rawProfiles, firefoxConfig, config);
        QCOMPARE(firefoxConfig->groupList().filter(QRegExp(R"(^Desktop Action new-(?:private-)?window-with-profile)")).size(), 4);
        manager.changeProfileRegistering(true, true, false, firefoxConfig);
        firefoxConfig->sync();
        QCOMPARE(getSplitCount(firefoxConfig->group("Desktop Entry").readEntry("Actions")), 6);

        // Unregister private windows
        manager.changeProfileRegistering(true, false, false, firefoxConfig);
        QCOMPARE(getSplitCount(firefoxConfig->group("Desktop Entry").readEntry("Actions")), 4);
        QCOMPARE(firefoxConfig->group("Desktop Entry").readEntry("Actions").split(";")
                         .filter(QRegExp(R"(^new-private-window-with-profile)")).count(), 0);

        // Unregister all windows
        manager.changeProfileRegistering(false, false, false, firefoxConfig);
        QCOMPARE(getSplitCount(firefoxConfig->group("Desktop Entry").readEntry("Actions")), 2);

        // Register all options
        manager.changeProfileRegistering(true, true, false, firefoxConfig);
        QCOMPARE(getSplitCount(firefoxConfig->group("Desktop Entry").readEntry("Actions")), 6);

        // Check if profiles are read correctly
        QList<Profile> customProfiles = manager.getCustomProfiles(firefoxConfig);
        QCOMPARE(customProfiles.count(), 2);
        for (const auto &profile:customProfiles) {
            if (profile.name == "Alex") {
                QVERIFY(profile.isDefault);
            }
        }
        QFile::remove(QDir::currentPath() + "/resources/_profiles_install.ini");
    }


    /**
     * Test if the plugin correctly reacts if profiles are added and deleted
     */
    static void testAddingAndDeletingOfProfiles() {
        QFile::copy(QDir::currentPath() + "/resources/profiles_install.ini", QDir::currentPath() + "/resources/_profiles_install.ini");

        ProfileManager manager;
        manager.firefoxProfilesIniPath = QDir::currentPath() + "/resources/_profiles_install.ini";
        manager.defaultPath = manager.getDefaultProfilePath();

        QFile::remove(QDir::currentPath() + "/resources/firefox-copy.desktop");
        QFile::copy(QDir::currentPath() + "/resources/firefox.desktop", QDir::currentPath() + "/resources/firefox-copy.desktop");
        manager.firefoxDesktopFile = QDir::currentPath() + "/resources/firefox-copy.desktop";
        const auto firefoxConfig = KSharedConfig::openConfig(manager.firefoxDesktopFile);
        firefoxConfig->reparseConfiguration();
        QList<Profile> rawProfiles = manager.getFirefoxProfiles();
        KConfigGroup _config = KSharedConfig::openConfig(QDir::currentPath() + "tmp")->group("Config");
        _config.config()->reparseConfiguration();

        // Check if Desktop Actions are created and if they are registered
        QCOMPARE(rawProfiles.size(), 2);
        manager.syncDesktopFile(rawProfiles, firefoxConfig, _config);
        firefoxConfig->sync();
        QCOMPARE(firefoxConfig->groupList().filter(QRegExp(R"(^Desktop Action new-(?:private-)?window-with-profile)")).size(), 4);
        manager.changeProfileRegistering(true, true, false, firefoxConfig);
        QCOMPARE(getSplitCount(firefoxConfig->group("Desktop Entry").readEntry("Actions")), 6);

        // Delete group and check if it and its registered actions has been removed
        firefoxConfig->deleteGroup("Desktop Action new-private-window-with-profile-snytc8pd.default-release-1");
        manager.changeProfileRegistering(false, true, false, firefoxConfig);
        QCOMPARE(getSplitCount(firefoxConfig->group("Desktop Entry").readEntry("Actions")), 3);
        QCOMPARE(firefoxConfig->group("Desktop Entry").readEntry("Actions").split(";")
                         .filter(QRegExp(R"(^new-private-window)")).count(), 2);

        // Add new profile
        QFile firefoxProfilesAppend(QDir::currentPath() + "/resources/_profiles_install.ini");
        if (firefoxProfilesAppend.open(QFile::Append)) {
            firefoxProfilesAppend.write("\n[Profile42]\nName=New Profile\nIsRelative=1\nPath=71lye2uh42.New Profile\n");
            firefoxProfilesAppend.close();
        }

        auto firefoxProfiles = manager.getFirefoxProfiles();
        QCOMPARE(firefoxProfiles.size(), 3);
        QCOMPARE(firefoxConfig->groupList().filter(QRegExp(R"(^Desktop Action new-(?:private-)?window-with-profile)")).size(), 4);
        manager.syncDesktopFile(firefoxProfiles, firefoxConfig, _config);
        auto newCustomProfiles = manager.getCustomProfiles(firefoxConfig);
        QCOMPARE(newCustomProfiles.size(), 3);
        manager.changeProfileRegistering(true, true, false, firefoxConfig);
        QCOMPARE(firefoxConfig->groupList().filter(QRegExp(R"(^Desktop Action new-)")).size(), 8);
        QCOMPARE(firefoxConfig->group("Desktop Entry").readEntry("Actions").split(";").count(), 8);
        QFile::remove(QDir::currentPath() + "/resources/_profiles_install.ini");
    }
};

QTEST_MAIN(ProfileManagerTest)
