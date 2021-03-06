cmake_minimum_required (VERSION 3.5 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# KDE Application Version, managed by release script
set (RELEASE_SERVICE_VERSION_MAJOR "20")
set (RELEASE_SERVICE_VERSION_MINOR "03")
set (RELEASE_SERVICE_VERSION_MICRO "70")
set (RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")

project(cantor VERSION ${RELEASE_SERVICE_VERSION})

set(KF5_MIN_VERSION "5.49.0")

find_package(ECM 5.15.0 REQUIRED CONFIG)
set(CMAKE_MODULE_PATH ${cantor_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

find_package(Qt5 5.6.0 CONFIG REQUIRED
    Core
    Widgets
    PrintSupport
    Svg
    Xml
    XmlPatterns
    Network
    Test)

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED
    Config
    Crash
    Completion
    DocTools
    NewStuff
    IconThemes
    TextEditor
    CoreAddons
    Archive
    Parts
    SyntaxHighlighting
    TextWidgets
    KIO
    XmlGui
    I18n)

find_package(Poppler "0.62.0" REQUIRED COMPONENTS Qt5)


if(NOT WIN32)
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED Pty)
endif()

include(FeatureSummary)
include(ECMInstallIcons)
include(ECMSetupVersion)
include(KDEInstallDirs)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings)
include(ECMAddAppIcon)
include(GenerateExportHeader)

add_subdirectory(thirdparty)
# Need to set this here, because variable can't move from subdirectory (thirdparty) to upper cmake scope
set(Discount_FOUND TRUE)

if(NOT WIN32)
    set_package_properties(LibSpectre PROPERTIES DESCRIPTION "A PostScript rendering library"
        URL "https://www.freedesktop.org/wiki/Software/libspectre/"
        TYPE OPTIONAL
        PURPOSE "Support for rendering EPS files in Cantor")

    find_package(LibSpectre)
    if(LIBSPECTRE_FOUND)
        set(WITH_EPS On)
    else(LIBSPECTRE_FOUND)
        set(WITH_EPS Off)
    endif(LIBSPECTRE_FOUND)

else(NOT WIN32)
    set(WITH_EPS Off)
endif(NOT WIN32)

#[[
find_package(Discount 2.2.0)
set_package_properties(Discount PROPERTIES DESCRIPTION "A C implementation of the Markdown markup language"
    URL "https://www.pell.portland.or.us/~orc/Code/discount/"
    TYPE OPTIONAL
    PURPOSE "Used for Markdown entries in Cantor")
]]#

add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
string(TOLOWER "${CMAKE_BUILD_TYPE}" BUILD_NAME)
if (BUILD_NAME STREQUAL "release" OR BUILD_NAME STREQUAL "")
    add_definitions(-DQT_NO_DEBUG_OUTPUT)
endif()
kde_enable_exceptions()

add_subdirectory(doc)
add_subdirectory(src)
add_subdirectory(icons)

set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/Cantor")
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/CantorConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/CantorConfig.cmake
    INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}/
    #PATH_VARS INCLUDE_INSTALL_DIR SYSCONFIG_INSTALL_DIR
)

ecm_setup_version(${RELEASE_SERVICE_VERSION}
    VARIABLE_PREFIX CANTOR
    VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/cantor_version.h"
    PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/CantorConfigVersion.cmake"
)
install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/CantorConfig.cmake
          ${CMAKE_CURRENT_BINARY_DIR}/CantorConfigVersion.cmake
    DESTINATION ${CMAKECONFIG_INSTALL_DIR}
    COMPONENT Devel
)

install(EXPORT CantorTargets
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    FILE CantorTargets.cmake
    NAMESPACE Cantor::
)


install(FILES org.kde.cantor.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})


feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
