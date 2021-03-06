cmake_minimum_required(VERSION 3.0)

set(REQUIRED_QT_VERSION 5.8.0)

set(CMAKE_CXX_STANDARD 11)

set(MAUIKIT_VERSION 0.0.5)

set(CMAKE_AUTOMOC ON)
set(AUTOMOC_MOC_OPTIONS -Muri=org.kde.maui)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

project(mauikit VERSION ${MAUIKIT_VERSION})

find_package(ECM 5.45.0 NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} 
    ${ECM_KDE_MODULE_DIR})

include(GenerateExportHeader)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(CMakePackageConfigHelpers)
include(ECMPoQmTools)
include(ECMQMLModules)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(ECMQtDeclareLoggingCategory)
include(ECMAddQch)
include(KDECompilerSettings NO_POLICY_SCOPE)

find_package(Qt5 ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE COMPONENTS Qml 
    Sql Core Quick Gui Svg QuickControls2 Network DBus Xml)
ecm_find_qmlmodule(QtGraphicalEffects 1.0)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}/src

    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils
    ${CMAKE_CURRENT_BINARY_DIR}/src/utils
    
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/tagging
    ${CMAKE_CURRENT_BINARY_DIR}/src/utils/tagging
    
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/editor
    ${CMAKE_CURRENT_BINARY_DIR}/src/utils/editor
    
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/syncing
    ${CMAKE_CURRENT_BINARY_DIR}/src/utils/syncing 
    
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/store
    ${CMAKE_CURRENT_BINARY_DIR}/src/utils/store
    
	${CMAKE_CURRENT_SOURCE_DIR}/src/utils/syncing/libwebdavclient/lib
	${CMAKE_CURRENT_SOURCE_DIR}/src/utils/syncing/libwebdavclient/lib/utils
	${CMAKE_CURRENT_SOURCE_DIR}/src/utils/syncing/libwebdavclient/lib/dto
    
    ${CMAKE_CURRENT_SOURCE_DIR}/src/fm
    ${CMAKE_CURRENT_BINARY_DIR}/src/fm

    ${CMAKE_CURRENT_SOURCE_DIR}/src/kde
    ${CMAKE_CURRENT_BINARY_DIR}/src/kde
    )

set(mauikit_SRCS
    src/mauikit.cpp
    src/utils/handy.cpp
    )

set(mauikit_HDRS
    src/mauikit.h
    src/utils.h
    src/utils/handy.h
    )

set(editor_SRCS
    src/utils/editor/documenthandler.cpp
    )

set(editor_HDRS
    src/utils/editor/documenthandler.cpp
    )

set(fm_SRCS
    src/fm/fm.cpp
    src/fm/fmdb.cpp
    src/fm/fmmodel.cpp
    src/fm/fmlist.cpp
    src/fm/placeslist.cpp
    src/fm/placesmodel.cpp
    )

set(fm_HDRS
    src/fm/fmh.h
    src/fm/fm.h
    src/fm/fmdb.h
    src/fm/fmmodel.h
    src/fm/fmlist.h
    src/fm/placeslist.h
    src/fm/placesmodel.h
    )

set(tagging_SRCS
    src/utils/tagging/tagging.cpp
    src/utils/tagging/tagdb.cpp
    src/utils/tagging/tagsmodel.cpp
    src/utils/tagging/tagslist.cpp
    )

set(tagging_HDRS
    src/utils/tagging/tag.h
    src/utils/tagging/tagging.h
    src/utils/tagging/tagdb.h
    src/utils/tagging/tagsmodel.h
    src/utils/tagging/tagslist.h
    )

set(syncing_SRCS
    src/utils/syncing/syncing.cpp
    src/utils/syncing/syncingmodel.cpp
    src/utils/syncing/syncinglist.cpp
    src/utils/syncing/libwebdavclient/lib/WebDAVClient.cpp
    src/utils/syncing/libwebdavclient/lib/dto/WebDAVItem.cpp
    src/utils/syncing/libwebdavclient/lib/utils/Environment.cpp
    src/utils/syncing/libwebdavclient/lib/utils/NetworkHelper.cpp
    src/utils/syncing/libwebdavclient/lib/utils/WebDAVReply.cpp
    src/utils/syncing/libwebdavclient/lib/utils/XMLHelper.cpp
    )

set(syncing_HDRS
    src/utils/syncing/syncing.h
    src/utils/syncing/syncingmodel.h
    src/utils/syncing/syncinglist.h
    src/utils/syncing/libwebdavclient/lib/WebDAVClient.hpp
    src/utils/syncing/libwebdavclient/lib/dto/WebDAVItem.hpp
    src/utils/syncing/libwebdavclient/lib/utils/Environment.hpp
    src/utils/syncing/libwebdavclient/lib/utils/NetworkHelper.hpp
    src/utils/syncing/libwebdavclient/lib/utils/WebDAVReply.hpp
    src/utils/syncing/libwebdavclient/lib/utils/XMLHelper.hpp
    )

set(store_SRCS
    src/utils/store/store.cpp
    src/utils/store/storemodel.cpp
    src/utils/store/storelist.cpp
    )

set(store_HDRS
    src/utils/store/store.h    
    src/utils/store/storemodel.h
    src/utils/store/storelist.h
    )


#use dbus on linux, bsd etc, but not andoid and apple stuff
if(ANDROID)

    find_package(Qt5 REQUIRED COMPONENTS AndroidExtras Xml WebView Network)
    qt5_add_resources(MauiAndroid_RESOURCES src/android/android.qrc src/android/icons.qrc)
    
    set(MAUIANDROID_LIBS
        Qt5::Xml
        Qt5::AndroidExtras
        Qt5::Network
        Qt5::WebView
        )

    if (NOT EXISTS ${CMAKE_SOURCE_DIR}/src/android/icons/luv-icon-theme/.git)
        find_package(Git REQUIRED)
        execute_process(COMMAND ${GIT_EXECUTABLE} clone --depth 1 https://github.com/Nitrux/luv-icon-theme.git ${CMAKE_SOURCE_DIR}/src/android/icons/luv-icon-theme)
    endif()
    set(mauikit_Android_SRCS
        src/android/mauiandroid.cpp
        ${MauiAndroid_RESOURCES}
        ${MauiAndroid_icons_RESOURCES}
        )
else()
    find_package(Qt5 REQUIRED COMPONENTS WebEngine)
    find_package(KF5 ${KF5_VERSION} REQUIRED COMPONENTS I18n Notifications
        Config Service KIO ConfigWidgets Attica)

    set(mauikit_KDE_SRCS
        src/kde/mauikde.cpp
        src/kde/kdeconnect.cpp
        )

    set(mauikit_KDE_HDRS
        src/kde/mauikde.h
        src/kde/kdeconnect.h
        )

    set(MAUIKDE_KF5LIBS
        KF5::ConfigCore
        KF5::Notifications
        KF5::I18n KF5::Service
        KF5::KIOCore
        KF5::KIOWidgets
        KF5::KIOFileWidgets
        KF5::Service
        KF5::KIONTLM
        KF5::ConfigWidgets
        KF5::Attica
        Qt5::WebEngine
        )
endif()
# set(CMAKE_AUTORCC ON)

qt5_add_resources(mauikit_ASSETS ${CMAKE_CURRENT_SOURCE_DIR}/assets.qrc)
qt5_add_resources(tagging_RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/tagging/tagging.qrc)
qt5_add_resources(fm_RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/fm/fm.qrc)
qt5_add_resources(store_RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/store/store.qrc)

# qt5_add_resources(mauikit_RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/mauikit.qrc)

add_library(MauiKit 

    ${fm_HDRS}
    ${fm_SRCS}
    ${fm_RESOURCES}

    ${editor_HDRS}
    ${editor_SRCS}

    ${syncing_HDRS}
    ${syncing_SRCS}
    
    ${store_SRCS}
    ${store_HDRS}
	${store_RESOURCES}
    
    ${tagging_HDRS}
    ${tagging_SRCS}
    ${tagging_RESOURCES}

    ${mauikit_HDRS}
    ${mauikit_SRCS}
    ${mauikit_RESOURCES}
    ${mauikit_ASSETS}

    ${mauikit_Android_SRCS}

    ${mauikit_KDE_HDRS}
    ${mauikit_KDE_SRCS}
    )

target_link_libraries(MauiKit
    PUBLIC
    Qt5::Core
    Qt5::Sql
    Qt5::Qml
    Qt5::Quick
    Qt5::QuickControls2
    Qt5::Svg
    Qt5::Gui
    Qt5::Network
    Qt5::Xml
    ${MAUIKDE_KF5LIBS}
    ${MAUIANDROID_LIBS}
# #     webdavclient
    )

if (ANDROID)
    kde_enable_exceptions(MauiKit PRIVATE)
    target_include_directories(MauiKit PRIVATE src/android)
    install(FILES src/android/mauiandroid.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/MauiKit COMPONENT Devel)

    install(DIRECTORY src/android/ DESTINATION ${KDE_INSTALL_DATAROOTDIR}/MauiKitAndroid COMPONENT Devel)
endif()

generate_export_header(MauiKit BASE_NAME MauiKit)
install(TARGETS MauiKit EXPORT MauiKitTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

target_include_directories(MauiKit 
    INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/MauiKit>")

add_custom_target(copy)

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin/org/kde/mauikit)
add_custom_command(TARGET copy PRE_BUILD COMMAND ${CMAKE_COMMAND} 
    -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/src/controls
    ${CMAKE_BINARY_DIR}/bin/org/kde/mauikit/)

add_dependencies(MauiKit copy)

install(DIRECTORY src/controls/ DESTINATION 
    ${KDE_INSTALL_QMLDIR}/org/kde/mauikit)

install(TARGETS MauiKit DESTINATION 
    ${KDE_INSTALL_QMLDIR}/org/kde/mauikit)

install(FILES
    ${mauikit_HDRS}
    ${mauikit_KDE_HDRS}
    ${tagging_HDRS}
    ${fm_HDRS}
    ${syncing_HDRS}
    ${store_HDRS}
    ${CMAKE_CURRENT_BINARY_DIR}/mauikit_export.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/MauiKit
    COMPONENT Devel)

##INSTALL MAUI STYLE
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/maui-style DESTINATION ${KDE_INSTALL_QMLDIR}/QtQuick/Controls.2)   

##CMAKE PARTS   

set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/MauiKit")

ecm_setup_version(${MAUIKIT_VERSION}
    VARIABLE_PREFIX MAUIKIT
    VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/mauikit_version.h"
    PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/MauiKitConfigVersion.cmake"
    SOVERSION 5
    )

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/MauiKitConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/MauiKitConfig.cmake"
    INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
    PATH_VARS  KF5_INCLUDE_INSTALL_DIR CMAKE_INSTALL_PREFIX
    )

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/MauiKitConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/MauiKitConfigVersion.cmake"
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    COMPONENT Devel
    )

install(EXPORT MauiKitTargets
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    FILE MauiKitTargets.cmake
    )
