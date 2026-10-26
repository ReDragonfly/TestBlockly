#pragma once

#include <QAtomicInt>
#include <QObject>
#include <QString>

struct lua_State;
class SignalGeneratorController;
class SpectrumAnalyzerController;
class DeviceController;
class IResultSink;

// Lua 脚本执行引擎：持有 lua_State，并向 Lua 注册业务函数。
// 本对象被移动到专用线程，所有 Lua 执行都通过 BlockingQueuedConnection 串行化到该线程。
class ScriptEngine : public QObject
{
    Q_OBJECT
public:
    ScriptEngine(SignalGeneratorController *sg,
                 SpectrumAnalyzerController *sa,
                 DeviceController *dev,
                 IResultSink *sink,
                 QObject *parent = nullptr);
    ~ScriptEngine() override;

    // 在引擎线程中执行一段 Lua 代码。
    // 返回："OK" / "ABORTED" / "ERROR: ..."
    Q_INVOKABLE QString executeInThread(const QString &luaCode);

    void requestAbort();
    void resetAbort();
    bool isAborted() const;

signals:
    void logMessage(const QString &message);
    void stepStarted(const QString &blockId);

private:
    void registerFunctions();

    lua_State *L = nullptr;

    SignalGeneratorController *m_sg = nullptr;
    SpectrumAnalyzerController *m_sa = nullptr;
    DeviceController *m_dev = nullptr;
    IResultSink *m_sink = nullptr;

    QAtomicInt m_abortFlag {0};

    // 辅助：从 Lua 注册表中取回 ScriptEngine 实例
    static ScriptEngine *instanceFromLua(lua_State *L);

    // Lua 可调用的 C++ 静态函数
    static int l_reportStep(lua_State *L);
    static int l_sleep(lua_State *L);
    static int l_showMsg(lua_State *L);
    static int l_setSgCW(lua_State *L);
    static int l_setSgRfOn(lua_State *L);
    static int l_setSgRfOff(lua_State *L);
    static int l_setSaCW(lua_State *L);
    static int l_getSaPower(lua_State *L);
    static int l_setOmtParam(lua_State *L);
    static int l_getOmtParam(lua_State *L);
    static int l_sendTestResult(lua_State *L);
};
