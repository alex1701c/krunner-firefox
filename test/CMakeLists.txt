find_package(Qt5Test REQUIRED)
enable_testing()

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Copy test resources
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources/profiles_install.ini DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/resources)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources/profiles.ini DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/resources)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources/firefox.desktop DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/resources)

set(test_includes_SRCS ProfileManagerTest.h ../src/profile/ProfileManager.cpp ../src/profile/Profile.cpp)

add_executable(profile_manager_test ${test_includes_SRCS})
add_test(profile_manager_test profile_manager_test)
target_link_libraries(profile_manager_test
        Qt5::Test
        Qt5::Core
        Qt5::Gui
        KF5::CoreAddons
        KF5::ConfigCore
        KF5::I18n
        KF5::ConfigWidgets
        KF5::Runner
        )
