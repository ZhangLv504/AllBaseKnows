cmake_minimum_required(VERSION 3.17)

project(QtKnowles LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------- Qt5 支持 ----------
if(NOT DEFINED CMAKE_PREFIX_PATH AND NOT DEFINED Qt5_DIR)
    set(CMAKE_PREFIX_PATH "E:/Software/Qt/5.15.2/msvc2019_64" CACHE PATH "Qt5 prefix")
endif()

find_package(Qt5 5.15.2 EXACT REQUIRED COMPONENTS
        Core
        Gui
        Widgets
        Sql)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# ---------- MSVC 编码 ----------
if(MSVC)
    add_compile_options(/utf-8)
    add_compile_options(/Zc:__cplusplus)
else()
    add_compile_options(-finput-charset=UTF-8)
endif()

link_directories("${CMAKE_SOURCE_DIR}/lib")

# ---------- 收集 src 目录下的源文件 ----------
# 使用 GLOB_RECURSE 递归扫描 src 文件夹，以后在 src 里建子文件夹也能自动包含
file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.ui"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.qrc"
)

add_executable(QtKnowles
        ${APP_SOURCES}
)

# 可执行文件与运行时依赖统一输出到交付目录 bin
set_target_properties(QtKnowles PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib"
)

# ---------- 包含目录 ----------
# 确保 src 本身在头文件搜索路径中，便于源码之间互相 #include 以及找到 AUTOUIC 生成的 ui_*.h
# `GLOB_RECURSE` 只回答「 哪些文件参与构建 」，
# `target_include_directories` 才回答「 `#include xxx` 时到哪些目录下找 xxx 」。
target_include_directories(QtKnowles PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/dbPack"
    "${CMAKE_SOURCE_DIR}/publics/include"
)

# ---------- 链接库 ----------
target_link_libraries(QtKnowles PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Sql
    QtMySQL
)
