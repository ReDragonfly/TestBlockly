#include "ScriptBridge.h"

#include "ScriptEngine.h"
#include "controllers/FrontendResultSink.h"

#include <QFile>
#include <QFileDialog>
#include <QFuture>
#include <QtConcurrent/QtConcurrentRun>

ScriptBridge::ScriptBridge(ScriptEngine *engine, FrontendResultSink *sink, QObject *parent)
    : QObject(parent)
    , m_engine(engine)
{
    // 引擎日志、步骤、结果三类信号统一转发到前端
    connect(m_engine, &ScriptEngine::logMessage, this, &ScriptBridge::logMessage);
    connect(m_engine, &ScriptEngine::stepStarted, this, &ScriptBridge::stepStarted);

    if (sink) {
        connect(sink, &FrontendResultSink::resultReported, this,
                &ScriptBridge::testResultReceived);
    }
}

QString ScriptBridge::submitScript(const QString &luaCode)
{
    const QString taskId = QStringLiteral("task_%1").arg(++m_taskCounter);
    emit logMessage(QStringLiteral("[C++] 收到 Lua 代码，长度: %1 字符").arg(luaCode.length()));

    // 后台线程发起，但通过 BlockingQueuedConnection 把真正的执行串行化到引擎线程，
    // 保证 Lua 状态机始终只在专用线程内被访问（见设计方案 7.4）。
    const QFuture<void> future = QtConcurrent::run([this, taskId, luaCode]() {
        QString result;
        QMetaObject::invokeMethod(m_engine, "executeInThread", Qt::BlockingQueuedConnection,
                                  Q_RETURN_ARG(QString, result),
                                  Q_ARG(QString, luaCode));
        emit executionFinished(taskId, result);
    });
    Q_UNUSED(future);

    return taskId;
}

void ScriptBridge::abort()
{
    if (m_engine)
        m_engine->requestAbort();
}

bool ScriptBridge::saveWorkspace(const QString &json)
{
    const QString path = QFileDialog::getSaveFileName(
        nullptr, QStringLiteral("保存工作区"),
        QString(), QStringLiteral("Blockly 工作区 (*.json)"));
    if (path.isEmpty())
        return false;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;
    file.write(json.toUtf8());
    return true;
}

QString ScriptBridge::loadWorkspace()
{
    const QString path = QFileDialog::getOpenFileName(
        nullptr, QStringLiteral("加载工作区"),
        QString(), QStringLiteral("Blockly 工作区 (*.json)"));
    if (path.isEmpty())
        return QString();

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return QString();
    return QString::fromUtf8(file.readAll());
}
