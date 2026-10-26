cmake_minimum_required(VERSION 3.16)

project(BlocklyPrototype LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Widgets WebEngineWidgets WebChannel Concurrent)

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
# 后端抽象层 + 控制器层 + 仿真器
# ---------------------------------------------------------------------------
set(CONTROLLER_SOURCES
    controllers/IInstrumentBackend.h
    controllers/IDeviceBackend.h
    controllers/IResultSink.h
    controllers/FrontendResultSink.h
    controllers/FrontendResultSink.cpp
    controllers/InstrumentBackendFactory.h
    controllers/InstrumentBackendFactory.cpp
    controllers/VisaBackend.h
    controllers/VisaBackend.cpp
    controllers/SimulatorBackend.h
    controllers/SimulatorBackend.cpp
    controllers/SignalGeneratorSimulator.h
    controllers/SignalGeneratorSimulator.cpp
    controllers/SpectrumAnalyzerSimulator.h
    controllers/SpectrumAnalyzerSimulator.cpp
    controllers/SimulatorDeviceBackend.h
    controllers/SimulatorDeviceBackend.cpp
    controllers/SignalGeneratorController.h
    controllers/SignalGeneratorController.cpp
    controllers/SpectrumAnalyzerController.h
    controllers/SpectrumAnalyzerController.cpp
    controllers/DeviceController.h
    controllers/DeviceController.cpp
)

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
    ConfigurationManager.h
    ConfigurationManager.cpp
    ui/SimulationPanel.h
    ui/SimulationPanel.cpp
    ${CONTROLLER_SOURCES}
    resources.qrc
)

target_include_directories(BlocklyPrototype PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/controllers"
    "${CMAKE_CURRENT_SOURCE_DIR}/ui"
)

target_link_libraries(BlocklyPrototype PRIVATE
    Qt6::Widgets
    Qt6::WebEngineWidgets
    Qt6::WebChannel
    Qt6::Concurrent
    lua54
)

if(WIN32)
    target_link_libraries(BlocklyPrototype PRIVATE ws2_32)
endif()

# ---------------------------------------------------------------------------
# 可选：NI-VISA（找到才定义 HAVE_VISA，否则仅编译仿真后端，见设计方案 T13）
# ---------------------------------------------------------------------------
find_path(VISA_INCLUDE_DIR NAMES visa.h)
find_library(VISA_LIBRARY NAMES visa32 visa)
if(VISA_INCLUDE_DIR AND VISA_LIBRARY)
    message(STATUS "找到 VISA 库: ${VISA_LIBRARY}")
    target_compile_definitions(BlocklyPrototype PRIVATE HAVE_VISA)
    target_include_directories(BlocklyPrototype PRIVATE ${VISA_INCLUDE_DIR})
    target_link_libraries(BlocklyPrototype PRIVATE ${VISA_LIBRARY})
else()
    message(STATUS "未找到 VISA 库，仅编译仿真后端")
endif()

# 原型阶段保留控制台，便于查看 C++ 侧日志（见方案 7.2 验证清单第 5 项）
set_target_properties(BlocklyPrototype PROPERTIES
    WIN32_EXECUTABLE FALSE
)
