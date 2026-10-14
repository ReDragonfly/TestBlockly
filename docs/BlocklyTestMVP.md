# 第一阶段原型验证详细设计方案

## 一、概述

### 1.1 设计目标

本方案的目标是构建一个**最小可行原型**，验证以下核心技术链路：

1. Qt C++ 应用能否通过 `QWebEngineView` 成功加载并显示 Blockly 编辑器。
2. Blockly 编辑器能否正确生成 Lua 代码字符串。
3. `QWebChannel` 能否在前端 JavaScript 与后端 C++ 之间建立可靠的双向通信。
4. C++ 端能否接收 Lua 代码并通过内嵌 Lua 解释器成功执行，调用 C++ 注册的底层函数。

原型阶段不追求积木体系完整或界面精美，核心是**打通技术链路**，为后续阶段的功能扩展提供经过验证的架构基础。

### 1.2 验证范围

| 验证项 | 具体内容 | 通过标准 |
|--------|---------|---------|
| Blockly 加载 | QWebEngineView 显示 Blockly 工作区 | 工作区可正常交互，工具箱可拖拽积木 |
| 积木生成 | 自定义积木生成正确的 Lua 代码 | 生成的 Lua 字符串语法正确 |
| 通信链路 | JS 调用 C++ 方法，C++ 回调 JS | 双向通信稳定，无数据丢失 |
| Lua 执行 | C++ 端执行 Lua 脚本并返回结果 | 脚本执行成功，C++ 注册函数被正确调用 |
| 端到端流程 | 从拖拽积木到执行结果的完整流程 | 点击“运行”按钮后，C++ 端输出预期结果 |

### 1.3 技术栈

| 层次 | 技术 | 版本要求 |
|------|------|---------|
| 宿主框架 | Qt C++ | Qt Qt 6.5.3 |
| Web 引擎 | Qt WebEngineWidgets | 随 Qt 安装 |
| 通信桥接 | Qt WebChannel | 随 Qt 安装 |
| 脚本引擎 | Lua 5.3/5.4 | 独立集成 |
| 前端框架 | Blockly | v10+ |
| 构建工具 | CMake | — |


## 二、系统架构

### 2.1 架构图

```
┌─────────────────────────────────────────────────────┐
│                   Qt C++ 宿主应用                     │
│                                                      │
│  ┌──────────────┐      ┌───────────────────────┐    │
│  │  MainWindow   │      │   ScriptEngine         │    │
│  │  (QWidget)    │      │   (QObject)            │    │
│  │               │      │                        │    │
│  │ ┌───────────┐ │      │  ┌─────────────────┐  │    │
│  │ │QWebEngine │ │      │  │ lua_State*      │  │    │
│  │ │View       │ │      │  │                 │  │    │
│  │ │           │ │      │  │ luaL_dostring() │  │    │
│  │ │ ┌───────┐ │ │      │  │ lua_register()  │  │    │
│  │ │ │Blockly│ │ │      │  └─────────────────┘  │    │
│  │ │ │Editor │ │ │      │                        │    │
│  │ │ └───────┘ │ │      └───────────────────────┘    │
│  │ └───────────┘ │              ▲                    │
│  │       ▲       │              │                    │
│  │       │       │     QWebChannel (桥接)             │
│  │       ▼       │              │                    │
│  │ ┌───────────┐ │              │                    │
│  │ │JavaScript │◄├──────────────┘                    │
│  │ │(前端逻辑) │ │                                   │
│  │ └───────────┘ │                                   │
│  └──────────────┘                                    │
└─────────────────────────────────────────────────────┘
```

### 2.2 数据流

```
用户拖拽积木 → Blockly 工作区
                    ↓
        luaGenerator.workspaceToCode(workspace)
                    ↓
              Lua 代码字符串
                    ↓
   JS 调用 window.scriptBridge.executeScript(luaCode)
                    ↓
        QWebChannel 序列化传输
                    ↓
   C++ ScriptEngine::executeScript(QString code)
                    ↓
          luaL_dostring(L, code)
                    ↓
   Lua 脚本调用 C++ 注册函数（如 setSgPower）
                    ↓
         C++ 执行实际硬件操作，结果通过信号返回
                    ↓
   JS 回调接收结果 → 更新 UI 显示
```


## 三、开发环境准备

### 3.1 Qt 项目配置

创建 Qt Widgets Application 项目，在 `.pro` 文件中添加必要模块：

```qmake
QT += core gui widgets webenginewidgets webchannel

# 如果使用 Lua 源码集成
INCLUDEPATH += $$PWD/lua/include
LIBS += -L$$PWD/lua/lib -llua

# Windows 平台可能需要额外配置
win32 {
    LIBS += -lws2_32
}

# 资源文件
RESOURCES += resources.qrc
```

如果使用 CMake：

```cmake
find_package(Qt6 REQUIRED COMPONENTS Widgets WebEngineWidgets WebChannel)
target_link_libraries(BlocklyPrototype PRIVATE
    Qt6::Widgets
    Qt6::WebEngineWidgets
    Qt6::WebChannel
)
```

### 3.2 Lua 引擎集成

将 Lua 源码（除 `lua.c` 和 `luac.c` 外）加入工程编译，或使用预编译库。建议使用 Lua 5.4 以获取最新的 API 和性能改进。

### 3.3 Blockly 前端资源准备

从 Blockly 官方 CDN 下载以下文件，放入项目 `blockly/` 目录：

- `blockly_compressed.js` — Blockly 核心库
- `lua_compressed.js` — Lua 代码生成器
- `blocks_compressed.js` — 标准积木库（可选，用于提供基础积木）

在 HTML 中通过 `<script>` 标签引入，注意**生成器必须在 Blockly 核心之后引入**。


## 四、Blockly 前端设计

### 4.1 目录结构

```
blockly/
├── index.html          # 主页面
├── blockly-config.js   # Blockly 初始化配置
├── custom-blocks.js    # 自定义积木定义
├── custom-generators.js# 自定义积木的 Lua 代码生成器
├── toolbox.js          # 工具箱配置
└── bridge.js           # QWebChannel 通信封装
```

### 4.2 HTML 页面骨架

创建 `index.html`，包含 Blockly 工作区容器、控制按钮和脚本引入：

```html
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <title>Blockly 自动化测试编辑器</title>
    <style>
        html, body { height: 100%; margin: 0; padding: 0; }
        #blocklyDiv { width: 100%; height: calc(100% - 50px); }
        #toolbar {
            height: 50px; display: flex; align-items: center;
            padding: 0 16px; background: #f5f5f5;
            border-bottom: 1px solid #ddd; gap: 12px;
        }
        #toolbar button {
            padding: 6px 16px; border: 1px solid #ccc;
            border-radius: 4px; cursor: pointer; background: #fff;
        }
        #toolbar button:hover { background: #e8e8e8; }
        #status { margin-left: auto; color: #666; font-size: 13px; }
    </style>
</head>
<body>
    <div id="toolbar">
        <button id="btnRun">▶ 运行</button>
        <button id="btnSave">保存工作区</button>
        <button id="btnLoad">加载工作区</button>
        <span id="status">就绪</span>
    </div>
    <div id="blocklyDiv"></div>

    <!-- Blockly 核心（必须在生成器之前引入） -->
    <script src="blockly/blockly_compressed.js"></script>
    <script src="blockly/blocks_compressed.js"></script>
    <script src="blockly/lua_compressed.js"></script>

    <!-- 项目自定义脚本 -->
    <script src="custom-blocks.js"></script>
    <script src="custom-generators.js"></script>
    <script src="toolbox.js"></script>
    <script src="bridge.js"></script>
    <script src="blockly-config.js"></script>
</body>
</html>
```

### 4.3 Blockly 初始化配置

创建 `blockly-config.js`，在 `DOMContentLoaded` 后注入工作区：

```javascript
let workspace;

document.addEventListener('DOMContentLoaded', function() {
    // 定义自定义积木
    defineCustomBlocks();

    // 注入 Blockly 工作区
    workspace = Blockly.inject('blocklyDiv', {
        toolbox: toolboxDefinition,       // 来自 toolbox.js
        trashcan: true,
        zoom: { controls: true, wheel: true, startScale: 0.9 },
        grid: { spacing: 20, length: 3, colour: '#ccc', snap: true },
        renderer: 'zelos'                 // 使用块状渲染器，更美观
    });

    // 注册 Lua 代码生成器
    registerLuaGenerators(luaGenerator);

    // 初始化 QWebChannel 通信
    initBridge();

    // 按钮事件绑定
    document.getElementById('btnRun').addEventListener('click', onRunClicked);
    document.getElementById('btnSave').addEventListener('click', onSaveClicked);
    document.getElementById('btnLoad').addEventListener('click', onLoadClicked);
});

function onRunClicked() {
    const luaCode = luaGenerator.workspaceToCode(workspace);
    if (!luaCode.trim()) {
        setStatus('工作区为空，请先拖入积木', 'orange');
        return;
    }
    setStatus('正在执行...', 'blue');
    // 通过 QWebChannel 发送给 C++
    if (window.scriptBridge) {
        window.scriptBridge.executeScript(luaCode, function(result) {
            if (result === 'OK') {
                setStatus('执行成功', 'green');
            } else {
                setStatus('执行失败: ' + result, 'red');
            }
        });
    }
}

function setStatus(text, color) {
    const el = document.getElementById('status');
    el.textContent = text;
    el.style.color = color || '#666';
}
```

### 4.4 自定义积木定义

创建 `custom-blocks.js`，原型阶段定义 3 个核心积木用于验证：

**积木 1：设置信号源功率**

```javascript
function defineCustomBlocks() {
    Blockly.common.defineBlocksWithJsonArray([
        {
            "type": "sg_set_power",
            "message0": "设置信号源功率 %1 dBm",
            "args0": [
                {
                    "type": "input_value",
                    "name": "POWER",
                    "check": "Number"
                }
            ],
            "previousStatement": null,
            "nextStatement": null,
            "colour": 230,
            "tooltip": "设置信号源的输出功率",
            "helpUrl": ""
        },
        {
            "type": "sa_get_power",
            "message0": "读取频谱仪功率",
            "output": "Number",
            "colour": 230,
            "tooltip": "从频谱仪读取当前功率值",
            "helpUrl": ""
        },
        {
            "type": "dev_set_omt_param",
            "message0": "设置设备参数 OID %1 值 %2",
            "args0": [
                {
                    "type": "field_input",
                    "name": "OID",
                    "text": "06D3"
                },
                {
                    "type": "input_value",
                    "name": "VALUE",
                    "check": "Number"
                }
            ],
            "previousStatement": null,
            "nextStatement": null,
            "colour": 160,
            "tooltip": "设置OMT参数",
            "helpUrl": ""
        }
    ]);
}
```

积木定义使用 JSON 格式，可通过 Blockly Developer Tools 可视化生成。

### 4.5 Lua 代码生成器

创建 `custom-generators.js`，为每个自定义积木编写 Lua 生成逻辑：

```javascript
function registerLuaGenerators(generator) {
    // 设置信号源功率
    generator.forBlock['sg_set_power'] = function(block, gen) {
        const power = gen.valueToCode(block, 'POWER', gen.ORDER_ATOMIC) || '0';
        return `setSgPower(${power})\n`;
    };

    // 读取频谱仪功率（值积木，返回表达式字符串）
    generator.forBlock['sa_get_power'] = function(block, gen) {
        return ['getSaPower()', gen.ORDER_FUNCTION_CALL];
    };

    // 设置设备 OMT 参数
    generator.forBlock['dev_set_omt_param'] = function(block, gen) {
        const oid = block.getFieldValue('OID');
        const value = gen.valueToCode(block, 'VALUE', gen.ORDER_ATOMIC) || '0';
        return `setOmtParam("${oid}", ${value})\n`;
    };
}
```

**生成结果示例**：当用户拖入“设置信号源功率 → 读取频谱仪功率 → 设置设备参数”并串联后，生成的 Lua 代码为：

```lua
setSgPower(-30)
getSaPower()
setOmtParam("06D3", 8191)
```

### 4.6 工具箱配置

创建 `toolbox.js`：

```javascript
const toolboxDefinition = {
    "contents": [
        {
            "kind": "category",
            "name": "仪器控制",
            "colour": "230",
            "contents": [
                { "kind": "block", "type": "sg_set_power" },
                { "kind": "block", "type": "sa_get_power" }
            ]
        },
        {
            "kind": "category",
            "name": "设备配置",
            "colour": "160",
            "contents": [
                { "kind": "block", "type": "dev_set_omt_param" }
            ]
        },
        {
            "kind": "category",
            "name": "逻辑",
            "colour": "210",
            "contents": [
                { "kind": "block", "type": "controls_if" },
                { "kind": "block", "type": "logic_compare" }
            ]
        },
        {
            "kind": "category",
            "name": "数学",
            "colour": "230",
            "contents": [
                { "kind": "block", "type": "math_number" },
                { "kind": "block", "type": "math_arithmetic" }
            ]
        }
    ]
};
```


## 五、QWebChannel 通信层

### 5.1 C++ 端暴露对象

创建 `ScriptBridge` 类，作为暴露给 JavaScript 的桥接对象：

```cpp
// ScriptBridge.h
#pragma once
#include <QObject>
#include <QString>

class ScriptEngine; // 前向声明

class ScriptBridge : public QObject
{
    Q_OBJECT
public:
    explicit ScriptBridge(ScriptEngine *engine, QObject *parent = nullptr);

    // Q_INVOKABLE 标记的方法可从 JavaScript 调用
    Q_INVOKABLE QString executeScript(const QString &luaCode);

signals:
    // 信号可被 JavaScript 监听
    void executionFinished(const QString &result);
    void logMessage(const QString &message);

private:
    ScriptEngine *m_engine;
};
```

```cpp
// ScriptBridge.cpp
#include "ScriptBridge.h"
#include "ScriptEngine.h"

ScriptBridge::ScriptBridge(ScriptEngine *engine, QObject *parent)
    : QObject(parent), m_engine(engine)
{
    // 将引擎的日志信号转发给前端
    connect(m_engine, &ScriptEngine::logMessage,
            this, &ScriptBridge::logMessage);
}

QString ScriptBridge::executeScript(const QString &luaCode)
{
    emit logMessage(QStringLiteral("[C++] 收到 Lua 代码，长度: %1 字符")
                        .arg(luaCode.length()));
    QString result = m_engine->execute(luaCode);
    emit executionFinished(result);
    return result;
}
```

### 5.2 前端 JavaScript 通信封装

创建 `bridge.js`，使用 Qt 提供的 `qwebchannel.js` 建立连接：

```javascript
let scriptBridge = null;

function initBridge() {
    if (typeof QWebChannel === 'undefined') {
        setStatus('QWebChannel 未加载，请检查 qrc 资源', 'red');
        return;
    }

    new QWebChannel(qt.webChannelTransport, function(channel) {
        scriptBridge = channel.objects.scriptBridge;
        window.scriptBridge = scriptBridge;

        // 监听 C++ 端信号
        scriptBridge.logMessage.connect(function(msg) {
            console.log('[C++ Log]', msg);
        });

        scriptBridge.executionFinished.connect(function(result) {
            console.log('[C++] 执行结果:', result);
        });

        setStatus('通信通道已就绪', 'green');
    });
}
```

**关键说明**：QWebChannel 的通信是**异步**的。`executeScript` 的返回值在 JavaScript 端需要通过回调函数获取，C++ 方法的返回值会自动传递给回调。此外，`qwebchannel.js` 文件需通过 `qrc:///qtwebchannel/qwebchannel.js` 路径引入（见 6.2 节资源文件配置）。

### 5.3 通信注意事项

- **异步特性**：JS 调用 C++ 方法是非阻塞的，如果 C++ 端处理耗时较长，建议先返回一个确认字符串，最终结果通过 `executionFinished` 信号回传。
- **数据类型**：仅支持可 JSON 序列化的类型（QString、int、double、bool、QVariantList、QVariantMap 等）。
- **调试建议**：在 C++ 端启用 `QWebEngineView::page()->setDevToolsPage()` 或设置 `QTWEBENGINE_REMOTE_DEBUGGING` 环境变量，在 Chrome 中调试前端。


## 六、C++ 宿主端实现

### 6.1 ScriptEngine — Lua 执行引擎

```cpp
// ScriptEngine.h
#pragma once
#include <QObject>
#include <QString>

struct lua_State;

class ScriptEngine : public QObject
{
    Q_OBJECT
public:
    explicit ScriptEngine(QObject *parent = nullptr);
    ~ScriptEngine();

    QString execute(const QString &luaCode);

signals:
    void logMessage(const QString &message);

private:
    lua_State *L = nullptr;

    void registerFunctions();
    void reportError(const QString &context);

    // Lua 可调用的 C++ 静态函数
    static int l_setSgPower(lua_State *L);
    static int l_getSaPower(lua_State *L);
    static int l_setOmtParam(lua_State *L);

    // 辅助：获取 ScriptEngine 实例
    static ScriptEngine *instanceFromLua(lua_State *L);
};
```

```cpp
// ScriptEngine.cpp
#include "ScriptEngine.h"
#include <QDebug>

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

ScriptEngine::ScriptEngine(QObject *parent)
    : QObject(parent)
{
    L = luaL_newstate();
    luaL_openlibs(L);
    registerFunctions();

    // 将 this 指针存入 Lua 注册表，供回调函数访问
    lua_pushlightuserdata(L, this);
    lua_setfield(L, LUA_REGISTRYINDEX, "ScriptEngine");
}

ScriptEngine::~ScriptEngine()
{
    if (L) {
        lua_close(L);
        L = nullptr;
    }
}

QString ScriptEngine::execute(const QString &luaCode)
{
    QByteArray utf8 = luaCode.toUtf8();
    int err = luaL_dostring(L, utf8.constData());
    if (err != LUA_OK) {
        const char *errMsg = lua_tostring(L, -1);
        QString msg = QString::fromUtf8(errMsg);
        lua_pop(L, 1);
        emit logMessage(QStringLiteral("[Lua 错误] %1").arg(msg));
        return QStringLiteral("ERROR: %1").arg(msg);
    }
    return QStringLiteral("OK");
}

void ScriptEngine::registerFunctions()
{
    lua_register(L, "setSgPower", l_setSgPower);
    lua_register(L, "getSaPower", l_getSaPower);
    lua_register(L, "setOmtParam", l_setOmtParam);
}

ScriptEngine *ScriptEngine::instanceFromLua(lua_State *L)
{
    lua_getfield(L, LUA_REGISTRYINDEX, "ScriptEngine");
    auto *engine = static_cast<ScriptEngine *>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    return engine;
}

// ---- C++ 注册到 Lua 的函数实现 ----

int ScriptEngine::l_setSgPower(lua_State *L)
{
    double power = luaL_checknumber(L, 1);
    auto *engine = instanceFromLua(L);
    if (engine) {
        engine->logMessage(QStringLiteral("[C++] setSgPower(%1) — 实际调用仪器VISA接口")
                               .arg(power));
        // TODO: 接入实际 VISA 控制代码
        // SgController::instance().setPower(power);
    }
    return 0;
}

int ScriptEngine::l_getSaPower(lua_State *L)
{
    auto *engine = instanceFromLua(L);
    double power = -30.0; // 原型阶段返回模拟值
    if (engine) {
        engine->logMessage(QStringLiteral("[C++] getSaPower() → 返回 %1 dBm")
                               .arg(power));
    }
    lua_pushnumber(L, power);
    return 1;
}

int ScriptEngine::l_setOmtParam(lua_State *L)
{
    const char *oid = luaL_checkstring(L, 1);
    int value = static_cast<int>(luaL_checknumber(L, 2));
    auto *engine = instanceFromLua(L);
    if (engine) {
        engine->logMessage(QStringLiteral("[C++] setOmtParam(\"%1\", %2)")
                               .arg(QString::fromUtf8(oid)).arg(value));
        // TODO: 接入实际 OMT 协议代码
        // OmtController::instance().setParam(oid, value);
    }
    return 0;
}
```

Lua 与 C++ 的数据交换需注意栈管理：`luaL_checknumber` 和 `luaL_checkstring` 从栈上读取参数，`lua_pushnumber` 和 `lua_pushstring` 将返回值压栈，返回值为压栈的元素个数。

### 6.2 MainWindow — WebEngine 与 QWebChannel 集成

```cpp
// MainWindow.h
#pragma once
#include <QMainWindow>

class QWebEngineView;
class ScriptEngine;
class ScriptBridge;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    void setupWebView();
    void setupWebChannel();

    QWebEngineView *m_webView = nullptr;
    ScriptEngine *m_scriptEngine = nullptr;
    ScriptBridge *m_scriptBridge = nullptr;
};
```

```cpp
// MainWindow.cpp
#include "MainWindow.h"
#include "ScriptEngine.h"
#include "ScriptBridge.h"

#include <QWebEngineView>
#include <QWebEnginePage>
#include <QWebEngineSettings>
#include <QWebChannel>
#include <QWebEngineUrlScheme>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 注册 qrc 方案，允许 Fetch API 访问资源（Blockly 内部需要）
    QWebEngineUrlScheme qrcScheme(QByteArrayLiteral("qrc"));
    qrcScheme.setFlags(QWebEngineUrlScheme::FetchApiAllowed);
    QWebEngineUrlScheme::registerScheme(qrcScheme);

    resize(1280, 800);
    setWindowTitle(QStringLiteral("Blockly 自动化测试 — 原型验证"));

    auto *central = new QWidget(this);
    auto *layout = new QVBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);

    m_webView = new QWebEngineView(this);
    layout->addWidget(m_webView);
    setCentralWidget(central);

    setupWebChannel();
    setupWebView();
}

void MainWindow::setupWebView()
{
    // 启用必要的 WebEngine 设置
    m_webView->settings()->setAttribute(
        QWebEngineSettings::LocalContentCanAccessRemoteUrls, true);
    m_webView->settings()->setAttribute(
        QWebEngineSettings::JavascriptEnabled, true);

    // 连接加载完成信号，用于调试
    connect(m_webView, &QWebEngineView::loadFinished,
            [](bool ok) {
        qDebug() << "[MainWindow] 页面加载" << (ok ? "成功" : "失败");
    });

    // 加载本地 Blockly 编辑器
    m_webView->load(QUrl(QStringLiteral("qrc:/blockly/index.html")));
}

void MainWindow::setupWebChannel()
{
    m_scriptEngine = new ScriptEngine(this);
    m_scriptBridge = new ScriptBridge(m_scriptEngine, this);

    auto *channel = new QWebChannel(this);
    // 注册对象名为 "scriptBridge"，前端通过 channel.objects.scriptBridge 访问
    channel->registerObject(QStringLiteral("scriptBridge"), m_scriptBridge);

    // 将 QWebChannel 绑定到 WebEngine 页面
    m_webView->page()->setWebChannel(channel);

    // 连接引擎日志到控制台输出
    connect(m_scriptEngine, &ScriptEngine::logMessage,
            [](const QString &msg) { qDebug() << msg; });
}
```

**关键点说明**：

- `QWebEngineUrlScheme` 注册 qrc 方案并启用 `FetchApiAllowed`，是因为 Blockly 内部可能使用 Fetch API 加载资源，默认 qrc 方案不支持该功能。此注册必须在任何 WebEngine 对象实例化**之前**完成。
- `QWebChannel` 通过 `setWebChannel()` 绑定到页面，注册的对象才能被前端 JavaScript 访问。
- `QUrl` 路径必须使用 `qrc:/blockly/index.html` 格式，不能省略 `qrc` 前缀。

### 6.3 资源文件配置

创建 `resources.qrc`，包含 Blockly 前端资源和 Qt 的 `qwebchannel.js`：

```xml
<RCC>
    <qresource prefix="/blockly">
        <file>blockly/index.html</file>
        <file>blockly/blockly_compressed.js</file>
        <file>blockly/lua_compressed.js</file>
        <file>blockly/blocks_compressed.js</file>
        <file>blockly/custom-blocks.js</file>
        <file>blockly/custom-generators.js</file>
        <file>blockly/toolbox.js</file>
        <file>blockly/bridge.js</file>
        <file>blockly/blockly-config.js</file>
    </qresource>
    <!-- Qt 自带的 qwebchannel.js，通过 qrc:///qtwebchannel/qwebchannel.js 访问 -->
</RCC>
```

在 `index.html` 的 `<head>` 中额外引入 qwebchannel.js：

```html
<script src="qrc:///qtwebchannel/qwebchannel.js"></script>
```

该文件由 Qt WebChannel 模块自动注册到资源系统，无需手动添加到 `.qrc` 文件中。


## 七、编译与测试

### 7.1 编译步骤

1. 确保 Qt 安装了 `WebEngineWidgets` 和 `WebChannel` 模块。使用 Qt Maintenance Tool 检查。
2. 下载 Blockly 压缩版 JS 文件放入 `blockly/` 目录。
3. 编译 Lua 源码或链接预编译库。
4. 执行 `qmake && make`（或 CMake 对应命令）。

### 7.2 验证清单

| 序号 | 操作 | 预期结果 |
|------|------|---------|
| 1 | 启动应用 | 窗口显示 Blockly 工作区，左侧显示工具箱 |
| 2 | 拖拽“设置信号源功率”积木到工作区 | 积木可拖拽，可编辑数值 |
| 3 | 拼接多个积木 | 积木可正常连接 |
| 4 | 点击“运行”按钮 | 状态栏显示“正在执行...”，随后显示“执行成功” |
| 5 | 查看 C++ 控制台 | 输出 `[C++] setSgPower(-30)` 等日志 |
| 6 | 组合“读取频谱仪功率”积木 | Lua 代码正确生成 `getSaPower()` |
| 7 | 拼接“设置设备参数”积木 | Lua 代码生成 `setOmtParam("06D3", 8191)` |
| 8 | 故意制造 Lua 语法错误 | 状态栏显示红色错误信息，C++ 不崩溃 |

### 7.3 常见问题与排查

| 现象 | 可能原因 | 解决方案 |
|------|---------|---------|
| 页面空白 | qrc 资源路径错误 | 确认 `QUrl("qrc:/blockly/index.html")` 路径与 qrc 前缀一致 |
| `QWebChannel is not defined` | `qwebchannel.js` 未引入 | 检查 `<script src="qrc:///qtwebchannel/qwebchannel.js">` |
| 页面加载但 Blockly 不显示 | `blockly_compressed.js` 加载失败 | 在 Chrome DevTools 中查看 Network 面板 |
| `executeScript` 回调不触发 | 对象名不匹配 | 检查 `registerObject("scriptBridge", ...)` 与前端 `channel.objects.scriptBridge` 一致 |
| Lua 执行报错 `attempt to call a nil value` | C++ 函数未注册 | 确认 `registerFunctions()` 在 `luaL_openlibs` 之后调用 |
| Fetch API 错误 | qrc 方案未注册 | 确认 `QWebEngineUrlScheme` 注册在 `QApplication` 创建之前 |


## 八、目录结构总览

```
BlocklyPrototype/
├── BlocklyPrototype.pro          # qmake 项目文件
├── resources.qrc                 # 资源文件
├── main.cpp                      # 入口
├── MainWindow.h / .cpp           # 主窗口，WebEngine + WebChannel 集成
├── ScriptEngine.h / .cpp         # Lua 执行引擎
├── ScriptBridge.h / .cpp         # QWebChannel 桥接对象
├── lua/                          # Lua 源码或库文件
│   ├── include/
│   └── lib/
└── blockly/                      # Blockly 前端资源
    ├── index.html
    ├── blockly_compressed.js
    ├── lua_compressed.js
    ├── blocks_compressed.js
    ├── custom-blocks.js
    ├── custom-generators.js
    ├── toolbox.js
    ├── bridge.js
    └── blockly-config.js
```

## 九、后续扩展方向

原型验证通过后，可沿以下方向扩展：

1. **积木体系扩展**：将现有 Lua 脚本中的 `Sg`、`Sa`、`Omt`、`Momt` 等模块的公开接口批量抽象为积木，按功能分类组织到工具箱中。
2. **工作区持久化**：使用 `Blockly.serialization.workspaces.save()` 将工作区状态序列化为 JSON，通过 QWebChannel 传递到 C++ 端保存到文件，加载时反向操作。
3. **动态工具箱**：根据当前连接的设备或测试项目，通过 JavaScript 动态生成工具箱内容。
4. **实时日志显示**：在 Blockly 页面下方添加日志面板，通过 QWebChannel 信号将 C++ 端的执行日志实时推送到前端。
5. **积木粒度优化**：根据用户使用反馈，将高频组合操作封装为复合积木（如“执行完整功率定标流程”），降低搭建复杂度。
6. **错误可视化**：当 Lua 执行出错时，解析错误信息并在 Blockly 界面中高亮对应的积木。