cmake_minimum_required(VERSION 3.0)

project(firefoxprofilerunner)

set(QT_MIN_VERSION 5.12.0)
set(KF5_MIN_VERSION 5.88.0)

add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x050f00)
add_definitions(-DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x055700)

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_MODULE_PATH})

# Find the required Libaries
find_package(Qt5 ${QT_MIN_VERSION} REQUIRED CONFIG COMPONENTS Widgets Core Quick QuickWidgets)
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS I18n Service Runner TextWidgets ConfigWidgets PlasmaQuick)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
#add_compile_definitions(status_dev)
include(FeatureSummary)

add_subdirectory(src)
#add_subdirectory(test)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

# CPack build
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Krunner plugin to launch firefox profiles")
set(CPACK_PACKAGE_NAME krunner-${CMAKE_PROJECT_NAME})
set(CPACK_PACKAGE_VERSION "1.4.0")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "alex1701c")
SET(CPACK_GENERATOR "DEB;RPM")
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_SOURCE_DIR}/preinst")
SET(CPACK_RPM_PRE_INSTALL_SCRIPT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/preinst")
INCLUDE(CPack)
