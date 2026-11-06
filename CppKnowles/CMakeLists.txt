cmake_minimum_required(VERSION 3.17)

project(CppKnowles LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------- MSVC 编码 ----------
if(MSVC)
    add_compile_options(/utf-8)
    add_compile_options(/Zc:__cplusplus)
else()
    add_compile_options(-finput-charset=UTF-8)
endif()

# ---------- 收集 src 目录下的源文件 ----------
# 使用 GLOB_RECURSE 递归扫描 src 文件夹，以后在 src 里建子文件夹也能自动包含
file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cxx"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
)

add_executable(CppKnowles
        ${APP_SOURCES}
)

# ---------- 包含目录 ----------
# 确保 src 本身在头文件搜索路径中，便于源码之间互相 #include
target_include_directories(CppKnowles PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)
