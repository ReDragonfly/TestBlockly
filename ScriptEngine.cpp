#include "ScriptEngine.h"

#include "controllers/DeviceController.h"
#include "controllers/IResultSink.h"
#include "controllers/SignalGeneratorController.h"
#include "controllers/SpectrumAnalyzerController.h"

#include <QThread>
#include <QtGlobal>
#include <QtMath>

extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}

ScriptEngine::ScriptEngine(SignalGeneratorController *sg,
                           SpectrumAnalyzerController *sa,
                           DeviceController *dev,
                           IResultSink *sink,
                           QObject *parent)
    : QObject(parent)
    , m_sg(sg)
    , m_sa(sa)
    , m_dev(dev)
    , m_sink(sink)
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

QString ScriptEngine::executeInThread(const QString &luaCode)
{
    if (!L)
        return QStringLiteral("ERROR: Lua 状态机未初始化");

    resetAbort();

    const QByteArray utf8 = luaCode.toUtf8();
    const int err = luaL_dostring(L, utf8.constData());
    if (err == LUA_OK)
        return QStringLiteral("OK");

    const char *errMsg = lua_tostring(L, -1);
    const QString msg = QString::fromUtf8(errMsg ? errMsg : "unknown error");
    lua_pop(L, 1);

    if (isAborted()) {
        emit logMessage(QStringLiteral("[Lua] 执行已中断"));
        return QStringLiteral("ABORTED");
    }

    emit logMessage(QStringLiteral("[Lua 错误] %1").arg(msg));
    return QStringLiteral("ERROR: %1").arg(msg);
}

void ScriptEngine::requestAbort()
{
    m_abortFlag.storeRelease(1);
}

void ScriptEngine::resetAbort()
{
    m_abortFlag.storeRelease(0);
}

bool ScriptEngine::isAborted() const
{
    return m_abortFlag.loadAcquire() != 0;
}

void ScriptEngine::registerFunctions()
{
    lua_register(L, "reportStep", l_reportStep);
    lua_register(L, "sleep", l_sleep);
    lua_register(L, "showMsg", l_showMsg);
    lua_register(L, "setSgCW", l_setSgCW);
    lua_register(L, "setSgRfOn", l_setSgRfOn);
    lua_register(L, "setSgRfOff", l_setSgRfOff);
    lua_register(L, "setSaCW", l_setSaCW);
    lua_register(L, "getSaPower", l_getSaPower);
    lua_register(L, "setOmtParam", l_setOmtParam);
    lua_register(L, "getOmtParam", l_getOmtParam);
    lua_register(L, "sendTestResult", l_sendTestResult);
}

ScriptEngine *ScriptEngine::instanceFromLua(lua_State *L)
{
    lua_getfield(L, LUA_REGISTRYINDEX, "ScriptEngine");
    auto *engine = static_cast<ScriptEngine *>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    return engine;
}

// ---------------------------------------------------------------------------
// 注册到 Lua 的业务函数实现
// 每个函数开头检查中断标志，用户点"停止"后最迟在下一次操作时抛出 Lua 错误。
// ---------------------------------------------------------------------------

int ScriptEngine::l_reportStep(lua_State *L)
{
    auto *engine = instanceFromLua(L);
    if (!engine)
        return 0;
    if (engine->isAborted())
        return luaL_error(L, "执行已中断");

    const char *id = luaL_checkstring(L, 1);
    emit engine->stepStarted(QString::fromUtf8(id));
    return 0;
}

int ScriptEngine::l_sleep(lua_State *L)
{
    auto *engine = instanceFromLua(L);
    if (!engine)
        return 0;

    const int ms = static_cast<int>(luaL_checknumber(L, 1));
    // 分片等待，保证中断能被及时响应
    for (int elapsed = 0; elapsed < ms; elapsed += 50) {
        if (engine->isAborted())
            return luaL_error(L, "执行已中断");
        QThread::msleep(static_cast<unsigned long>(qMin(50, ms - elapsed)));
    }
    return 0;
}

int ScriptEngine::l_showMsg(lua_State *L)
{
    auto *engine = instanceFromLua(L);
    if (!engine)
        return 0;
    if (engine->isAborted())
        return luaL_error(L, "执行已中断");

    const char *msg = luaL_checkstring(L, 1);
    emit engine->logMessage(QString::fromUtf8(msg));
    return 0;
}

int ScriptEngine::l_setSgCW(lua_State *L)
{
    auto *engine = instanceFromLua(L);
    if (!engine)
        return 0;
    if (engine->isAborted())
        return luaL_error(L, "执行已中断");

    const double freq = luaL_checknumber(L, 1);
    const double loss = luaL_checknumber(L, 2);
    const double power = luaL_checknumber(L, 3);
    if (engine->m_sg)
        engine->m_sg->setSgCW(freq, loss, power);
    return 0;
}

int ScriptEngine::l_setSgRfOn(lua_State *L)
{
    auto *engine = instanceFromLua(L);
    if (!engine)
        return 0;
    if (engine->isAborted())
        return luaL_error(L, "执行已中断");

    if (engine->m_sg)
        engine->m_sg->setSgRfOn();
    return 0;
}

int ScriptEngine::l_setSgRfOff(lua_State *L)
{
    auto *engine = instanceFromLua(L);
    if (!engine)
        return 0;
    if (engine->isAborted())
        return luaL_error(L, "执行已中断");

    if (engine->m_sg)
        engine->m_sg->setSgRfOff();
    return 0;
}

int ScriptEngine::l_setSaCW(lua_State *L)
{
    auto *engine = instanceFromLua(L);
    if (!engine)
        return 0;
    if (engine->isAborted())
        return luaL_error(L, "执行已中断");

    const double freq = luaL_checknumber(L, 1);
    const double span = luaL_checknumber(L, 2);
    const double ref = luaL_checknumber(L, 3);
    if (engine->m_sa)
        engine->m_sa->setSaCW(freq, span <= 0.0 ? 10.0 : span, ref);
    return 0;
}

int ScriptEngine::l_getSaPower(lua_State *L)
{
    auto *engine = instanceFromLua(L);
    if (!engine)
        return 0;
    if (engine->isAborted())
        return luaL_error(L, "执行已中断");

    double power = engine->m_sa ? engine->m_sa->getSaPower() : qQNaN();
    if (!qIsFinite(power)) {
        emit engine->logMessage(QStringLiteral("[C++] getSaPower() 读取失败，返回底噪 -100 dBm"));
        power = -100.0;
    }
    lua_pushnumber(L, power);
    return 1;
}

int ScriptEngine::l_setOmtParam(lua_State *L)
{
    auto *engine = instanceFromLua(L);
    if (!engine)
        return 0;
    if (engine->isAborted())
        return luaL_error(L, "执行已中断");

    const char *oid = luaL_checkstring(L, 1);
    const int value = static_cast<int>(qRound(luaL_checknumber(L, 2)));
    if (engine->m_dev)
        engine->m_dev->setOmtParam(QString::fromUtf8(oid), value);
    return 0;
}

int ScriptEngine::l_getOmtParam(lua_State *L)
{
    auto *engine = instanceFromLua(L);
    if (!engine)
        return 0;
    if (engine->isAborted())
        return luaL_error(L, "执行已中断");

    const char *oid = luaL_checkstring(L, 1);
    const int value = engine->m_dev ? engine->m_dev->getOmtParam(QString::fromUtf8(oid)) : 0;
    lua_pushnumber(L, value);
    return 1;
}

int ScriptEngine::l_sendTestResult(lua_State *L)
{
    auto *engine = instanceFromLua(L);
    if (!engine)
        return 0;
    if (engine->isAborted())
        return luaL_error(L, "执行已中断");

    const QString link   = QString::fromUtf8(luaL_checkstring(L, 1));
    const QString item   = QString::fromUtf8(luaL_checkstring(L, 2));
    const QString cond   = QString::fromUtf8(luaL_checkstring(L, 3));
    const QString target = QString::fromUtf8(luaL_checkstring(L, 4));
    const QString value  = QString::fromUtf8(luaL_checkstring(L, 5));
    const QString unit   = QString::fromUtf8(luaL_checkstring(L, 6));
    const QString status = QString::fromUtf8(luaL_checkstring(L, 7));

    if (engine->m_sink)
        engine->m_sink->report(link, item, cond, target, value, unit, status);

    emit engine->logMessage(QStringLiteral("[结果] %1 / %2 = %3 %4 → %5")
                                .arg(link, item, value, unit, status));
    return 0;
}
