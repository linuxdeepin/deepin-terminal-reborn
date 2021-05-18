
project(deepin-terminal-test)

configure_file(../src/environments.h.in environments.h @ONLY)

# 设置添加gocv相关信息的输出
set(CMAKE_CXX_FLAGS "-g -fprofile-arcs -ftest-coverage")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(PROJECT_NAME_TEST ${PROJECT_NAME})

add_compile_options(-fno-access-control)

find_package(Qt5Test REQUIRED)
find_package(GTest REQUIRED)

add_definitions(${Qt5Test_DEFINITIONS})
add_definitions(-D UNIT_TEST)

include_directories(${GTEST_INCLUDE_DIRS})
include_directories("src/common")
include_directories("src/customcommand")
include_directories("src/encodeplugin")
include_directories("src/remotemanage")
include_directories("src/views")
include_directories("src/main")
include_directories("src/stub")
include_directories("src/3rdparty/terminalwidget/lib/")
include_directories("src/3rdparty/terminalwidget/lib/history")
include_directories("src/3rdparty/terminalwidget/lib/history/compact")

FILE(GLOB allSource
    ../src/common/*.cpp
    ../src/customcommand/*.cpp
    ../src/encodeplugin/*.cpp
    ../src/remotemanage/*.cpp
    ../src/settings/*.cpp
    ../src/views/*.cpp
    ../src/main/service.cpp
    ../src/main/windowsmanager.cpp
    ../src/main/mainwindow.cpp
    ../src/main/terminalapplication.cpp
    ../src/main/termproperties.cpp
    ../src/main/dbusmanager.cpp
    ../src/main/atspidesktop.cpp
)

FILE(GLOB allHeaders
    ../src/common/*.h
    ../src/customcommand/*.h
    ../src/encodeplugin/*.h
    ../src/main/*.h
    ../src/remotemanage/*.h
    ../src/settings/*.h
    ../src/views/*.h
)

FILE(GLOB allThirdPartySource
    ../3rdparty/terminalwidget/lib/*.cpp
    ../3rdparty/terminalwidget/lib/history/*.cpp
    ../3rdparty/terminalwidget/lib/history/compact/*.cpp
)

FILE(GLOB qrcFiles
    ../src/assets/resources.qrc
)

FILE(GLOB allTestHeader
    src/common/*.h
    src/customcommand/*.h
    src/encodeplugin/*.h
    src/main/*.h
    src/remotemanage/*.h
    src/settings/*.h
    src/views/*.h
    src/3rdparty/terminalwidget/lib/*.h
)

FILE(GLOB allTestSource
    src/common/*.cpp
    src/customcommand/*.cpp
    src/encodeplugin/*.cpp
    src/main/*.cpp
    src/remotemanage/*.cpp
    src/settings/*.cpp
    src/views/*.cpp
)

set(UI ../3rdparty/terminalwidget/lib/SearchBar.ui)
qt5_wrap_ui(UI_SRCS ${UI})

set(KB_LAYOUT_DIR "${CMAKE_INSTALL_FULL_DATADIR}/${TERMINALWIDGET_LIBRARY_NAME}/kb-layouts")
set(COLORSCHEMES_DIR "${CMAKE_INSTALL_FULL_DATADIR}/${TERMINALWIDGET_LIBRARY_NAME}/color-schemes")
set(TRANSLATIONS_DIR "${CMAKE_INSTALL_FULL_DATADIR}/${TERMINALWIDGET_LIBRARY_NAME}/translations")

add_definitions("-DHAVE_POSIX_OPENPT")
add_definitions("-DKB_LAYOUT_DIR=\"${KB_LAYOUT_DIR}\"")
add_definitions("-DCOLORSCHEMES_DIR=\"${COLORSCHEMES_DIR}\"")
add_definitions("-DTRANSLATIONS_DIR=\"${TRANSLATIONS_DIR}\"")

add_executable(${PROJECT_NAME_TEST}
    ${UI_SRCS}
    ${allThirdPartySource}
    ${allHeaders}
    ${allSource}
    ${allTestHeader}
    ${allTestSource}
    ${qrcFiles}
    ${uiFiles}
)

target_include_directories(${PROJECT_NAME_TEST} PUBLIC ${Qt5Widgets_LIBRARIES}
                                                        ${Qt5DBus_LIBRARIES}
                                                        ${Qt5TestLib_LIBRARIES}
                                                        ${Qt5Widgets_PRIVATE_INCLUDE_DIRS}
                                                        ${Qt5Core_PRIVATE_INCLUDE_DIRS}
                                                        ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
                                                        ${PROJECT_BINARY_DIR}
                                                        ${DtkWidget_INCLUDE_DIRS}
                                                        ${DtkCore_LIBRARIES}
                                                        ${DtkGUI_INCLUDE_DIRS}
                                                        ${DFrameworkDBus_INCLUDE_DIRS}
                                                        ${GOBJECT_INCLUDE_DIRS}
                                                        ${LIBSECRET_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME_TEST} PUBLIC ${LINK_LIBS} Qt5::Test ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} -lpthread -lm)

#------------------------------ 创建'make test'指令---------------------------------------
add_custom_target(test
#    #执行mkdir -p 创建 coverageResult
#    COMMAND mkdir -p coverageResult
#    #设置工作目录
#    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_custom_command(TARGET test
    COMMAND ${CMAKE_BINARY_DIR}/tests/${PROJECT_NAME_TEST}
)

#'make test'命令依赖与我们的测试程序
add_dependencies(test ${PROJECT_NAME_TEST})

#ASAN安全性检测标志
set(CMAKE_SAFETYTEST "${CMAKE_SAFETYTEST_ARG}")
if(CMAKE_SAFETYTEST STREQUAL "")
    set(CMAKE_SAFETYTEST "CMAKE_SAFETYTEST_ARG_OFF")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D${CMAKE_SAFETYTEST}")
if(CMAKE_SAFETYTEST STREQUAL "CMAKE_SAFETYTEST_ARG_ON")
  #安全测试选项
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fsanitize=address -O2")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fsanitize=address -O2")
endif()

