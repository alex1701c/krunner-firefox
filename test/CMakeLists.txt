find_package(Qt${QT_MAJOR_VERSION}Test REQUIRED)
include(ECMAddTests)

# Copy test resources
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources/profiles_install.ini DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/resources)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources/profiles.ini DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/resources)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources/firefox.desktop DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/resources)

ecm_add_test(ProfileManagerTest.cpp TEST_NAME profile_manager_test)
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
