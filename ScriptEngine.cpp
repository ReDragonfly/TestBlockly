#include "ScriptEngine.h"

#include <QDebug>

extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}

ScriptEngine::ScriptEngine(QObject *parent)
    : QObject(parent)
{
    L = luaL_newstate();
    luaL_openlibs(L);
    registerFunctions();

    // 将 this 指针存入 Lua 注册表，供回调函数访问宿主对象
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
    if (!L)
        return QStringLiteral("ERROR: Lua 状态机未初始化");

    const QByteArray utf8 = luaCode.toUtf8();
    const int err = luaL_dostring(L, utf8.constData());
    if (err != LUA_OK) {
        const char *errMsg = lua_tostring(L, -1);
        const QString msg = QString::fromUtf8(errMsg ? errMsg : "unknown error");
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

void ScriptEngine::reportError(const QString &context)
{
    const char *errMsg = L ? lua_tostring(L, -1) : nullptr;
    const QString msg = QString::fromUtf8(errMsg ? errMsg : "unknown error");
    if (L)
        lua_pop(L, 1);
    emit logMessage(QStringLiteral("[Lua 错误] %1: %2").arg(context, msg));
}

ScriptEngine *ScriptEngine::instanceFromLua(lua_State *L)
{
    lua_getfield(L, LUA_REGISTRYINDEX, "ScriptEngine");
    auto *engine = static_cast<ScriptEngine *>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    return engine;
}

// ---------------------------------------------------------------------------
// 注册到 Lua 的底层函数实现
// ---------------------------------------------------------------------------

int ScriptEngine::l_setSgPower(lua_State *L)
{
    const double power = luaL_checknumber(L, 1);
    if (auto *engine = instanceFromLua(L)) {
        engine->logMessage(QStringLiteral("[C++] setSgPower(%1) — 实际调用仪器 VISA 接口")
                               .arg(power));
        // TODO: 接入实际 VISA 控制代码
        // SgController::instance().setPower(power);
    }
    return 0;
}

int ScriptEngine::l_getSaPower(lua_State *L)
{
    const double power = -30.0; // 原型阶段返回模拟值
    if (auto *engine = instanceFromLua(L)) {
        engine->logMessage(QStringLiteral("[C++] getSaPower() → 返回 %1 dBm").arg(power));
    }
    lua_pushnumber(L, power);
    return 1;
}

int ScriptEngine::l_setOmtParam(lua_State *L)
{
    const char *oid = luaL_checkstring(L, 1);
    const int value = static_cast<int>(luaL_checknumber(L, 2));
    if (auto *engine = instanceFromLua(L)) {
        engine->logMessage(QStringLiteral("[C++] setOmtParam(\"%1\", %2)")
                               .arg(QString::fromUtf8(oid))
                               .arg(value));
        // TODO: 接入实际 OMT 协议代码
        // OmtController::instance().setParam(oid, value);
    }
    return 0;
}
