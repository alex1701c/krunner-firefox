add_definitions(-DTRANSLATION_DOMAIN=\"plasma_runner_org.kde.firefoxprofilerunner\")

set(core_SRCS profile/Profile.cpp profile/ProfileManager.cpp)

set(firefoxprofilerunner_SRCS firefoxprofilerunner.cpp ${core_SRCS})
add_library(krunner_firefoxprofilerunner MODULE ${firefoxprofilerunner_SRCS})
target_link_libraries(krunner_firefoxprofilerunner KF5::Runner KF5::I18n)

set(kcm_krunner_firefoxprofilerunner_SRCS ${core_SRCS} config/firefoxprofilerunner_config.cpp)
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

install(TARGETS krunner_firefoxprofilerunner DESTINATION ${KDE_INSTALL_QTPLUGINDIR}/kf5/krunner/)
install(TARGETS kcm_krunner_firefoxprofilerunner DESTINATION ${KDE_INSTALL_QTPLUGINDIR})

install(FILES plasma-runner-firefoxprofilerunner.desktop config/plasma-runner-firefoxprofilerunner_config.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})

if(KF5Runner_VERSION VERSION_LESS 5.72.0)
    install(FILES plasma-runner-firefoxprofilerunner.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
else()
    kcoreaddons_desktop_to_json(krunner_firefoxprofilerunner plasma-runner-firefoxprofilerunner.desktop)
endif()
