cmake_minimum_required(VERSION 3.16)

project(BlocklyPrototype LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Widgets WebEngineWidgets WebChannel)

# ---------------------------------------------------------------------------
# Lua 5.4.6 源码集成（排除 lua.c 解释器入口与 luac.c 编译器入口）
# ---------------------------------------------------------------------------
file(GLOB LUA_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/lua/*.c")
list(FILTER LUA_SOURCES EXCLUDE REGEX "/lua\\.c$")
list(FILTER LUA_SOURCES EXCLUDE REGEX "/luac\\.c$")

add_library(lua54 STATIC ${LUA_SOURCES})
target_include_directories(lua54 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/lua")

if(MSVC)
    target_compile_definitions(lua54 PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_compile_options(lua54 PRIVATE /W0)
endif()

# ---------------------------------------------------------------------------
# 宿主应用
# ---------------------------------------------------------------------------
qt_add_executable(BlocklyPrototype
    main.cpp
    MainWindow.h
    MainWindow.cpp
    ScriptEngine.h
    ScriptEngine.cpp
    ScriptBridge.h
    ScriptBridge.cpp
    resources.qrc
)

target_include_directories(BlocklyPrototype PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(BlocklyPrototype PRIVATE
    Qt6::Widgets
    Qt6::WebEngineWidgets
    Qt6::WebChannel
    lua54
)

if(WIN32)
    target_link_libraries(BlocklyPrototype PRIVATE ws2_32)
endif()

# 原型阶段保留控制台，便于查看 C++ 侧日志（见方案 7.2 验证清单第 5 项）
set_target_properties(BlocklyPrototype PROPERTIES
    WIN32_EXECUTABLE FALSE
)
