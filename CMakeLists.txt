project(deepin-terminal)

cmake_minimum_required(VERSION 3.9.5)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})

include (src/DtkSettingsToolsMacros)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(QT_MINIMUM_VERSION "5.7.1")

if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "sw_64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mieee")
endif ()

#compile flags
if (CMAKE_BUILD_TYPE MATCHES Debug)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Wextra")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wextra")

  # Enable Qt builtin debug mode
  add_definitions("-DQT_MESSAGELOGCONTEXT")
else()
  # -Wl, -O2 Enable linker optimizations
  # -Wl, --gc-sections
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O2 -Wl,-O1 -Wl,--gc-sections")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O2 -Wl,-O1 -Wl,--gc-sections")
endif()

#Use deepin-turbo for Performance optimization
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -Wl,--as-need -fPIE")
set(CMAKE_EXE_LINKER_FLAGS "-pie")

configure_file(src/environments.h.in environments.h @ONLY)

# Find the QtWidgets library

find_package(PkgConfig REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5DBus REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5LinguistTools REQUIRED)
find_package(Qt5Network REQUIRED)
#find_package(DtkWidget REQUIRED)
#find_package(DtkCore REQUIRED)
#find_package(DtkGui REQUIRED)

include(FindPkgConfig)

pkg_search_module(DtkWidget REQUIRED dtkwidget)
pkg_search_module(DtkGui REQUIRED dtkgui)
pkg_search_module(DtkCore REQUIRED dtkcore)
pkg_search_module(GOBJECT REQUIRED gobject-2.0)
pkg_check_modules(ATSPI2 REQUIRED atspi-2)
pkg_check_modules(DFrameworkDBus REQUIRED dframeworkdbus)
pkg_check_modules(LIBSECRET REQUIRED libsecret-1)


# Compile definitions for TerminalWidget
# So we can use QT_VERSION_CHECK
set(TERMINALWIDGET_VERSION_MAJOR "0")
set(TERMINALWIDGET_VERSION_MINOR "14")
set(TERMINALWIDGET_VERSION_PATCH "1")

add_subdirectory(3rdparty/terminalwidget)

include_directories(${DtkWidget_INCLUDE_DIRS})
include_directories(${DtkGui_INCLUDE_DIRS})
include_directories(${DtkCore_INCLUDE_DIRS})

include_directories("src/common")
include_directories("src/customcommand")
include_directories("src/encodeplugin")
include_directories("src/main")
include_directories("src/remotemanage")
include_directories("src/settings")
include_directories("src/theme")
include_directories("src/views")
include_directories("3rdparty/terminalwidget/lib")

set(LINK_LIBS
    Qt5::Core
    Qt5::DBus
    Qt5::Widgets
    Qt5::WidgetsPrivate
    Qt5::Network

    ${DtkWidget_LIBRARIES}
    ${DtkCore_LIBRARIES}
    ${DtkGUI_LIBRARIES}
    ${DFrameworkDBus_LIBRARIES}
    ${ATSPI2_LIBRARIES}
    ${GOBJECT_LIBRARIES}
    ${LIBSECRET_LIBRARIES}
)

# Populate a CMake variable with the sources
# TODO: portable headers?
set (DTNG_CPP_FILES
    src/common/utils.cpp
    src/common/settingio.cpp
    src/customcommand/customcommandoptdlg.cpp
    src/customcommand/customcommandpanel.cpp
    src/customcommand/customcommandplugin.cpp
    src/customcommand/customcommandsearchrstpanel.cpp
    src/customcommand/customcommandtoppanel.cpp
    src/encodeplugin/encodepanelplugin.cpp
    src/encodeplugin/encodepanel.cpp
    src/encodeplugin/encodelistview.cpp
    src/encodeplugin/encodelistmodel.cpp
    src/main/main.cpp
    src/main/service.cpp
    src/main/windowsmanager.cpp
    src/main/mainwindow.cpp
    src/main/terminalapplication.cpp
    src/main/termproperties.cpp    
    src/main/dbusmanager.cpp
    src/main/atspidesktop.cpp
    src/remotemanage/remotemanagementpanel.cpp
    src/remotemanage/remotemanagementplugn.cpp
    src/remotemanage/remotemanagementsearchpanel.cpp
    src/remotemanage/remotemanagementtoppanel.cpp
    src/remotemanage/serverconfiggrouppanel.cpp
    src/remotemanage/serverconfigmanager.cpp
    src/remotemanage/serverconfigoptdlg.cpp
    src/settings/newdspinbox.cpp
    src/settings/settings.cpp
    src/settings/settings_translation.cpp
    src/settings/shortcutmanager.cpp
    src/theme/themeitemdelegate.cpp
    src/theme/themelistmodel.cpp
    src/theme/themelistview.cpp
    src/theme/themepanel.cpp
    src/theme/themepanelplugin.cpp
    src/views/commonpanel.cpp
    src/views/myiconbutton.cpp
    src/views/operationconfirmdlg.cpp
    src/views/pagesearchbar.cpp
    src/views/rightpanel.cpp
    src/views/tabbar.cpp
    src/views/termwidget.cpp
    src/views/termwidgetpage.cpp
    src/views/termbasedialog.cpp
    src/views/terminputdialog.cpp
    src/views/termcommandlinkbutton.cpp
    src/views/titlebar.cpp
    src/views/itemwidget.cpp
    src/views/focusframe.cpp
    src/views/iconbutton.cpp
    src/views/listview.cpp
)

set (DTNG_HEADER_FILES
    src/common/define.h
    src/common/utils.h
    src/common/settingio.h
    src/customcommand/customcommandoptdlg.h
    src/customcommand/customcommandpanel.h
    src/customcommand/customcommandplugin.h
    src/customcommand/customcommandsearchrstpanel.h
    src/customcommand/customcommandtoppanel.h
    src/encodeplugin/encodepanelplugin.h
    src/encodeplugin/encodepanel.h
    src/encodeplugin/encodelistview.h
    src/encodeplugin/encodelistmodel.h
    src/main/windowsmanager.h
    src/main/mainwindow.h
    src/main/mainwindowplugininterface.h
    src/main/service.h
    src/main/terminalapplication.h
    src/main/termproperties.h
    src/main/dbusmanager.h
    src/main/atspidesktop.h
    src/remotemanage/remotemanagementpanel.h
    src/remotemanage/remotemanagementplugn.h
    src/remotemanage/remotemanagementsearchpanel.h
    src/remotemanage/remotemanagementtoppanel.h
    src/remotemanage/serverconfiggrouppanel.h
    src/remotemanage/serverconfigmanager.h
    src/remotemanage/serverconfigoptdlg.h
    src/settings/newdspinbox.h
    src/settings/settings.h
    src/settings/shortcutmanager.h
    src/theme/themeitemdelegate.h
    src/theme/themelistmodel.h
    src/theme/themelistview.h
    src/theme/themepanel.h
    src/theme/themepanelplugin.h
    src/views/commonpanel.h
    src/views/myiconbutton.h
    src/views/operationconfirmdlg.h
    src/views/pagesearchbar.h
    src/views/rightpanel.h
    src/views/tabbar.h
    src/views/termwidget.h
    src/views/termwidgetpage.h
    src/views/titlebar.h
    src/views/termbasedialog.h
    src/views/terminputdialog.h
    src/views/termcommandlinkbutton.h
    src/views/itemwidget.h
    src/views/focusframe.h
    src/views/iconbutton.h
    src/views/listview.h
)

set (DTNG_QRC_FILES
    src/assets/resources.qrc
)

# Program
set (EXE_NAME deepin-terminal)

# Translation
file (GLOB DTNG_TS_FILES translations/*.ts)
set (DTNG_CPP_FILES_FOR_I18N ${DTNG_CPP_FILES})

dtk_create_i18n_from_json(DTNG_SETTINGS_I18N_FILES default-config.json settings_translation.cpp)
list (APPEND DTNG_CPP_FILES_FOR_I18N ${DTNG_SETTINGS_I18N_FILES})
qt5_create_translation(DTNG_QM_FILES ${DTNG_CPP_FILES_FOR_I18N} ${DTNG_TS_FILES})

add_executable (${EXE_NAME}
    ${DTNG_HEADER_FILES}
    ${DTNG_CPP_FILES}
    ${DTNG_QRC_FILES}
    ${DTNG_QM_FILES}
)

target_include_directories(${EXE_NAME} PUBLIC ${Qt5Widgets_LIBRARIES}
                                              ${Qt5DBus_LIBRARIES}
                                              ${Qt5Widgets_PRIVATE_INCLUDE_DIRS}
                                              ${Qt5Core_PRIVATE_INCLUDE_DIRS}
                                              ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
                                              ${PROJECT_BINARY_DIR}
                                              ${DtkWidget_INCLUDE_DIRS}
                                              ${DtkCore_LIBRARIES}
                                              ${DtkGUI_INCLUDE_DIRS}
                                              ${DFrameworkDBus_INCLUDE_DIRS}
                                              ${ATSPI2_INCLUDE_DIRS}
                                              ${GOBJECT_INCLUDE_DIRS}
                                              ${LIBSECRET_INCLUDE_DIRS})

target_link_libraries (deepin-terminal ${LINK_LIBS} terminalwidget5)

# Install settings

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()

# Installation

#install(TARGETS deepin-terminal DESTINATION ${CMAKE_INSTALL_PREFIX}/share/deepin-terminal)

install(FILES src/deepin-terminal.desktop
        DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications/)

install(FILES src/assets/logo/deepin-terminal.svg
        DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/scalable/apps/)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/deepin-terminal
        DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
        PERMISSIONS OWNER_READ OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE)

install(FILES ${DTNG_QM_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/deepin-terminal/translations)
