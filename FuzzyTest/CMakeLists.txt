
cmake_minimum_required(VERSION 3.9.5)
project(FuzzyTest)

set(EXE_ATSPI_NAME atspi_test)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_C_COMPILER /usr/bin/clang)
set(CMAKE_CXX_COMPILER /usr/bin/clang++)

include(FindPkgConfig)
pkg_check_modules(MOD_ATSPI2 REQUIRED atspi-2)
pkg_check_modules(MOD_GOBJECT REQUIRED gobject-2.0)

find_package(Qt5 COMPONENTS Core Widgets Gui REQUIRED)


file(GLOB_RECURSE SRC_LIST FOLLOW_SYMLINKS LIST_DIRECTORIES false ${PROJECT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE INC_LIST FOLLOW_SYMLINKS LIST_DIRECTORIES false ${PROJECT_SOURCE_DIR}/src/*.h)
file(GLOB_RECURSE HEAD_LIST FOLLOW_SYMLINKS LIST_DIRECTORIES false ${PROJECT_SOURCE_DIR}/include/*.h)
list(APPEND INC_LIST ${HEAD_LIST})

add_executable(${EXE_ATSPI_NAME} ${SRC_LIST})

target_include_directories(${EXE_ATSPI_NAME} 
    PRIVATE Qt5::Core Qt5::Widgets Qt5::Gui)

target_link_libraries(${EXE_ATSPI_NAME} 
    PRIVATE Qt5::Core Qt5::Widgets Qt5::Gui)

target_compile_options(${EXE_ATSPI_NAME} 
    PRIVATE -g -std=c++11)

target_link_options(${EXE_ATSPI_NAME} 
    PRIVATE -fsanitize=undefined,address,leak -fsanitize=fuzzer)

target_include_directories(${EXE_ATSPI_NAME} 
    PRIVATE ${MOD_ATSPI2_INCLUDE_DIRS}
    PRIVATE ${MOD_GOBJECT_INCLUDE_DIRS}
    )

target_link_libraries(${EXE_ATSPI_NAME} 
    PRIVATE ${MOD_ATSPI2_LIBRARIES} 
    PRIVATE ${MOD_GOBJECT_LIBRARIES} 
    PRIVATE libFuzzer.a 
    PRIVATE pthread)
target_link_directories(${EXE_ATSPI_NAME} 
    PRIVATE ${MOD_ATSPI2_LIBRARY_DIRS}
    PRIVATE ${MOD_GOBJECT_LIBRARY_DIRS}
    PRIVATE ${PROJECT_SOURCE_DIR}/thirdlib)



