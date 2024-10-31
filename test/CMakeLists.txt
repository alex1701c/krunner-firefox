find_package(Qt${QT_MAJOR_VERSION}Test REQUIRED)
include(ECMAddTests)

ecm_add_test(ProfileManagerTest.cpp TEST_NAME profile_manager_test)
set_tests_properties(profile_manager_test PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
target_link_libraries(profile_manager_test
    Qt::Test
    Qt::Core
    Qt::Gui
    KF${QT_MAJOR_VERSION}::CoreAddons
    KF${QT_MAJOR_VERSION}::ConfigCore
    KF${QT_MAJOR_VERSION}::I18n
    KF${QT_MAJOR_VERSION}::Runner
    core_STATIC
)
