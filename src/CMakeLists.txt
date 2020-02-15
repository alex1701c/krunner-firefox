add_definitions(-DTRANSLATION_DOMAIN=\"plasma_runner_org.kde.firefoxprofilerunner\")

set(firefoxprofilerunner_SRCS firefoxprofilerunner.cpp helper.h profile/Profile.cpp profile/Profile.h profile/ProfileManager.cpp profile/ProfileManager.h)

add_library(krunner_firefoxprofilerunner MODULE ${firefoxprofilerunner_SRCS})
target_link_libraries(krunner_firefoxprofilerunner KF5::Runner KF5::I18n)

set(kcm_krunner_firefoxprofilerunner_SRCS
        config/firefoxprofilerunner_config.cpp helper.h
        profile/Profile.cpp profile/Profile.h profile/ProfileManager.cpp profile/ProfileManager.h)

ki18n_wrap_ui(kcm_krunner_firefoxprofilerunner_SRCS config/firefoxprofilerunner_config.ui)
add_library(kcm_krunner_firefoxprofilerunner MODULE ${kcm_krunner_firefoxprofilerunner_SRCS})
target_link_libraries(kcm_krunner_firefoxprofilerunner
        Qt5::Core
        Qt5::Gui
        KF5::CoreAddons
        KF5::ConfigCore
        KF5::I18n
        KF5::ConfigWidgets
        KF5::Runner
        )

add_dependencies(krunner_firefoxprofilerunner kcm_krunner_firefoxprofilerunner)


install(TARGETS krunner_firefoxprofilerunner kcm_krunner_firefoxprofilerunner DESTINATION ${KDE_INSTALL_QTPLUGINDIR})
install(FILES plasma-runner-firefoxprofilerunner.desktop config/plasma-runner-firefoxprofilerunner_config.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
