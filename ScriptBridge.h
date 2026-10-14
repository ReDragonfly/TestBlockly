#pragma once

#include <QObject>
#include <QString>

class ScriptEngine;

// 通过 QWebChannel 暴露给前端 JavaScript 的桥接对象
// 前端访问方式：channel.objects.scriptBridge
class ScriptBridge : public QObject
{
    Q_OBJECT
public:
    explicit ScriptBridge(ScriptEngine *engine, QObject *parent = nullptr);

    // Q_INVOKABLE 标记的方法可从 JavaScript 调用
    Q_INVOKABLE QString executeScript(const QString &luaCode);

signals:
    // 以下信号可被 JavaScript 监听
    void executionFinished(const QString &result);
    void logMessage(const QString &message);

private:
    ScriptEngine *m_engine = nullptr;
};
