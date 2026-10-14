#include "ScriptBridge.h"

#include "ScriptEngine.h"

ScriptBridge::ScriptBridge(ScriptEngine *engine, QObject *parent)
    : QObject(parent)
    , m_engine(engine)
{
    // 将引擎日志信号转发给前端
    connect(m_engine, &ScriptEngine::logMessage, this, &ScriptBridge::logMessage);
}

QString ScriptBridge::executeScript(const QString &luaCode)
{
    emit logMessage(QStringLiteral("[C++] 收到 Lua 代码，长度: %1 字符").arg(luaCode.length()));

    const QString result = m_engine->execute(luaCode);

    emit executionFinished(result);
    return result;
}
