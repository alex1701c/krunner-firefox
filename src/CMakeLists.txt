add_definitions(-DTRANSLATION_DOMAIN=\"plasma_runner_org.kde.firefoxprofilerunner\")

add_library(core_STATIC STATIC profile/Profile.cpp profile/ProfileManager.cpp)
target_link_libraries(core_STATIC PUBLIC KF${QT_MAJOR_VERSION}::ConfigCore)
set_target_properties(core_STATIC PROPERTIES POSITION_INDEPENDENT_CODE ON)

kcoreaddons_add_plugin(firefoxprofilerunner SOURCES firefoxprofilerunner.cpp INSTALL_NAMESPACE "kf${QT_MAJOR_VERSION}/krunner")
target_link_libraries(firefoxprofilerunner
    Qt::Core
    Qt::Widgets
    KF${QT_MAJOR_VERSION}::Runner
    KF${QT_MAJOR_VERSION}::I18n
    KF${QT_MAJOR_VERSION}::Service
    core_STATIC
)

set(kcm_firefoxprofilerunner_SRCS ${core_SRCS} config/firefoxprofilerunner_config.cpp)
kcoreaddons_add_plugin(kcm_firefoxprofilerunner SOURCES ${kcm_firefoxprofilerunner_SRCS} INSTALL_NAMESPACE "krunner/kcms")
ki18n_wrap_ui(kcm_firefoxprofilerunner config/firefoxprofilerunner_config.ui)
target_link_libraries(kcm_firefoxprofilerunner
    Qt::Core
    Qt::Gui
    KF${QT_MAJOR_VERSION}::CoreAddons
    KF${QT_MAJOR_VERSION}::ConfigCore
    KF${QT_MAJOR_VERSION}::I18n
    KF${QT_MAJOR_VERSION}::KCMUtils
    KF${QT_MAJOR_VERSION}::Runner
    core_STATIC
)
