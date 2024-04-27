find_package(Qt${QT_MAJOR_VERSION}Test REQUIRED)
enable_testing()

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Copy test resources
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources/profiles_install.ini DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/resources)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources/profiles.ini DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/resources)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources/firefox.desktop DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/resources)

set(test_includes_SRCS ProfileManagerTest.cpp ../src/profile/ProfileManager.cpp ../src/profile/Profile.cpp)

add_executable(profile_manager_test ${test_includes_SRCS})
add_test(profile_manager_test profile_manager_test)
target_link_libraries(profile_manager_test
    Qt::Test
    Qt::Core
    Qt::Gui
    KF${QT_MAJOR_VERSION}::CoreAddons
    KF${QT_MAJOR_VERSION}::ConfigCore
    KF${QT_MAJOR_VERSION}::I18n
    KF${QT_MAJOR_VERSION}::Runner
)
