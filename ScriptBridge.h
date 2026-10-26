#pragma once

#include <QObject>
#include <QString>

class ScriptEngine;
class FrontendResultSink;

// 通过 QWebChannel 暴露给前端 JavaScript 的桥接对象
// 前端访问方式：channel.objects.scriptBridge
class ScriptBridge : public QObject
{
    Q_OBJECT
public:
    ScriptBridge(ScriptEngine *engine, FrontendResultSink *sink, QObject *parent = nullptr);

    // 提交一段 Lua 代码执行，立即返回任务 id（异步执行，结果通过 executionFinished 通知）
    Q_INVOKABLE QString submitScript(const QString &luaCode);
    // 请求中断当前执行
    Q_INVOKABLE void abort();
    // 通过文件对话框保存/加载工作区 JSON
    Q_INVOKABLE bool    saveWorkspace(const QString &json);
    Q_INVOKABLE QString loadWorkspace();

signals:
    // 以下信号可被 JavaScript 监听
    void executionFinished(const QString &taskId, const QString &result);
    void logMessage(const QString &message);
    void testResultReceived(const QString &link, const QString &item, const QString &cond,
                            const QString &target, const QString &value, const QString &unit,
                            const QString &status);
    void stepStarted(const QString &blockId);

private:
    ScriptEngine *m_engine = nullptr;
    int m_taskCounter = 0;
};
