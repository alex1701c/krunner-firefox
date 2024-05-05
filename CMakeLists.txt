cmake_minimum_required(VERSION 3.16)

project(firefoxprofilerunner)

set(QT_MIN_VERSION 5.15.0)
set(KF_MIN_VERSION 5.105.0)
set(KDE_COMPILERSETTINGS_LEVEL "5.82.0")

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_MODULE_PATH})

include(QtVersionOption)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEClangFormat)
include(ECMDeprecationSettings)
include(ECMQtDeclareLoggingCategory)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(FeatureSummary)

find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} REQUIRED CONFIG COMPONENTS Core Widgets)
find_package(KF${QT_MAJOR_VERSION} ${KF_MIN_VERSION} REQUIRED COMPONENTS I18n Service Runner KCMUtils)

ecm_set_disabled_deprecation_versions(
	QT ${QT_MIN_VERSION}
	KF ${KF_MIN_VERSION}
)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

add_subdirectory(src)
if(BUILD_TESTING)
    add_subdirectory(test)
endif()

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

# add clang-format target for all our real source files
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})

