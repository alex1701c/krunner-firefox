#include "../src/profile/ProfileManager.h"
#include <KConfigGroup>
#include <KSharedConfig>
#include <QRegularExpression>
#include <QTemporaryFile>
#include <QTest>

inline int getSplitCount(const QString str)
{
    return str.split(";", Qt::SkipEmptyParts).count();
}

class ProfileManagerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
    }

    /**
     * Test if the profiles are read correctly
     * The profiles file that is read contains an Install.* group
     */
    static void testReadProfilesFromWithInstallGroup()
    {
        ProfileManager manager;
        manager.firefoxProfilesIniPath = QFINDTESTDATA("resources/profiles_install.ini");
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
    static void testReadProfilesFromWithoutInstallGroup()
    {
        ProfileManager manager;
        manager.firefoxProfilesIniPath = QFINDTESTDATA("resources/profiles.ini");
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
    static void testSyncAndReadInstalledProfile()
    {
        ProfileManager manager;
        manager.firefoxProfilesIniPath = QFINDTESTDATA("resources/profiles_install.ini");
        manager.defaultPath = manager.getDefaultProfilePath();

        QFile source(QFINDTESTDATA("resources/firefox.desktop"));
        source.open(QIODevice::ReadOnly);

        QTemporaryFile file;
        file.open();
        file.write(source.readAll());
        file.close();
        const auto firefoxConfig = KSharedConfig::openConfig(file.fileName(), KConfig::NoGlobals);
        QList<Profile> rawProfiles = manager.getFirefoxProfiles();
        KConfigGroup config = KSharedConfig::openConfig("tmpconfig")->group("Config");

        // Check if Desktop Actions are created and if they are registered
        QCOMPARE(rawProfiles.size(), 2);
        manager.syncDesktopFile(rawProfiles, firefoxConfig, config);
        QCOMPARE(firefoxConfig->groupList().filter(QRegularExpression(R"(^Desktop Action new-(?:private-)?window-with-profile)")).size(), 4);
        manager.changeProfileRegistering(true, true, firefoxConfig);
        firefoxConfig->sync();
        QCOMPARE(getSplitCount(firefoxConfig->group("Desktop Entry").readEntry("Actions")), 6);

        // Unregister private windows
        manager.changeProfileRegistering(true, false, firefoxConfig);
        QCOMPARE(getSplitCount(firefoxConfig->group("Desktop Entry").readEntry("Actions")), 4);
        QCOMPARE(
            firefoxConfig->group("Desktop Entry").readEntry("Actions").split(";").filter(QRegularExpression(R"(^new-private-window-with-profile)")).count(),
            0);

        // Unregister all windows
        manager.changeProfileRegistering(false, false, firefoxConfig);
        QCOMPARE(getSplitCount(firefoxConfig->group("Desktop Entry").readEntry("Actions")), 2);

        // Register all options
        manager.changeProfileRegistering(true, true, firefoxConfig);
        QCOMPARE(getSplitCount(firefoxConfig->group("Desktop Entry").readEntry("Actions")), 6);

        // Check if profiles are read correctly
        QList<Profile> customProfiles = manager.getCustomProfiles(firefoxConfig);
        QCOMPARE(customProfiles.count(), 2);
        for (const auto &profile : customProfiles) {
            if (profile.name == "Alex") {
                QVERIFY(profile.isDefault);
            }
        }
    }

    /**
     * Test if the plugin correctly reacts if profiles are added and deleted
     */
    static void testAddingAndDeletingOfProfiles()
    {
        QFile sourceFile(QFINDTESTDATA("resources/profiles_install.ini"));
        QVERIFY(sourceFile.open(QIODevice::ReadOnly));
        QTemporaryFile profilesFile;
        profilesFile.open();
        profilesFile.write(sourceFile.readAll());
        profilesFile.close();

        QFile source(QFINDTESTDATA("resources/firefox.desktop"));
        source.open(QIODevice::ReadOnly);
        QTemporaryFile desktopFile;
        QVERIFY(desktopFile.open());
        desktopFile.write(source.readAll());
        desktopFile.close();

        ProfileManager manager;
        manager.firefoxProfilesIniPath = profilesFile.fileName();
        manager.defaultPath = manager.getDefaultProfilePath();
        manager.firefoxDesktopFile = desktopFile.fileName();

        const auto firefoxConfig = KSharedConfig::openConfig(manager.firefoxDesktopFile);
        firefoxConfig->reparseConfiguration();
        QList<Profile> rawProfiles = manager.getFirefoxProfiles();
        KConfigGroup _config = KSharedConfig::openConfig("tmp")->group("Config");
        _config.config()->reparseConfiguration();

        // Check if Desktop Actions are created and if they are registered
        QCOMPARE(rawProfiles.size(), 2);
        manager.syncDesktopFile(rawProfiles, firefoxConfig, _config);
        firefoxConfig->sync();
        QCOMPARE(firefoxConfig->groupList().filter(QRegularExpression(R"(^Desktop Action new-(?:private-)?window-with-profile)")).size(), 4);
        manager.changeProfileRegistering(true, true, firefoxConfig);
        QCOMPARE(getSplitCount(firefoxConfig->group("Desktop Entry").readEntry("Actions")), 6);

        // Delete group and check if it and its registered actions has been removed
        firefoxConfig->deleteGroup("Desktop Action new-private-window-with-profile-snytc8pd.default-release-1");
        manager.changeProfileRegistering(false, true, firefoxConfig);
        QCOMPARE(getSplitCount(firefoxConfig->group("Desktop Entry").readEntry("Actions")), 3);
        QCOMPARE(firefoxConfig->group("Desktop Entry").readEntry("Actions").split(";").filter(QRegularExpression(R"(^new-private-window)")).count(), 2);

        // Add new profile
        {
            QFile firefoxProfilesAppend(profilesFile.fileName());
            QVERIFY(firefoxProfilesAppend.open(QFile::Append));
            firefoxProfilesAppend.write("\n[Profile42]\nName=New Profile\nIsRelative=1\nPath=71lye2uh42.New Profile\n");
        }

        auto firefoxProfiles = manager.getFirefoxProfiles();
        QCOMPARE(firefoxProfiles.size(), 3);
        QCOMPARE(firefoxConfig->groupList().filter(QRegularExpression(R"(^Desktop Action new-(?:private-)?window-with-profile)")).size(), 3);

        manager.syncDesktopFile(firefoxProfiles, firefoxConfig, _config);
        auto newCustomProfiles = manager.getCustomProfiles(firefoxConfig);
        QCOMPARE(newCustomProfiles.size(), 3);
        manager.changeProfileRegistering(true, true, firefoxConfig);
        qWarning() << firefoxConfig->groupList().filter(QRegularExpression(R"(^Desktop Action new-)"));
        QCOMPARE(firefoxConfig->groupList().filter(QRegularExpression(R"(^Desktop Action new-)")).size(), 7);
        QCOMPARE(firefoxConfig->group("Desktop Entry").readEntry("Actions").split(";", Qt::SkipEmptyParts).count(), 7);
    }
};

QTEST_MAIN(ProfileManagerTest)

#include "ProfileManagerTest.moc"
