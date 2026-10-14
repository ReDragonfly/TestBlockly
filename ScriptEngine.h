#pragma once

#include <QObject>
#include <QString>

struct lua_State;

// Lua 脚本执行引擎：持有 lua_State，并向 Lua 注册底层 C++ 函数
class ScriptEngine : public QObject
{
    Q_OBJECT
public:
    explicit ScriptEngine(QObject *parent = nullptr);
    ~ScriptEngine() override;

    // 执行一段 Lua 代码，成功返回 "OK"，失败返回 "ERROR: ..."
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

    // 辅助：从 Lua 注册表中取回 ScriptEngine 实例
    static ScriptEngine *instanceFromLua(lua_State *L);
};
