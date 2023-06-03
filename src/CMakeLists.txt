add_definitions(-DTRANSLATION_DOMAIN=\"plasma_runner_org.kde.firefoxprofilerunner\")

set(core_SRCS profile/Profile.cpp profile/ProfileManager.cpp)

set(firefoxprofilerunner_SRCS firefoxprofilerunner.cpp ${core_SRCS})
kcoreaddons_add_plugin(firefoxprofilerunner SOURCES ${firefoxprofilerunner_SRCS} INSTALL_NAMESPACE "kf5/krunner")
target_link_libraries(firefoxprofilerunner KF5::Runner KF5::I18n Qt5::Widgets)

set(kcm_firefoxprofilerunner_SRCS ${core_SRCS} config/firefoxprofilerunner_config.cpp)
kcoreaddons_add_plugin(kcm_firefoxprofilerunner SOURCES ${kcm_firefoxprofilerunner_SRCS} INSTALL_NAMESPACE "kf5/krunner/kcms")
ki18n_wrap_ui(kcm_firefoxprofilerunner config/firefoxprofilerunner_config.ui)
target_link_libraries(kcm_firefoxprofilerunner
    Qt5::Core
    Qt5::Gui
    KF5::CoreAddons
    KF5::ConfigCore
    KF5::I18n
    KF5::ConfigWidgets
    KF5::Runner
)
