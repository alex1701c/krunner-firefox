find_package(Qt5Test REQUIRED)
enable_testing()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(profile_manager_test ProfileManagerTest.h)
add_test(profile_manager_test profile_manager_test)

target_link_libraries(profile_manager_test Qt5::Test)